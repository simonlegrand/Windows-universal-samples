//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// Scenario3.xaml.cpp
// Implementation of the Scenario3 class
//

#include "pch.h"
#include "Robuffer.h"
#include "wrl.h"
#include "Scenario3.xaml.h"
#include <iostream>
#include <sstream>
#include <string>
#include <locale>
#include <codecvt>

using namespace SDKTemplate::StreamSocketSample;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::System;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Microsoft::WRL;

Scenario3::Scenario3()
{
    InitializeComponent();
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void Scenario3::OnNavigatedTo(NavigationEventArgs^ e)
{
    // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
    // as NotifyUser()
    rootPage = MainPage::Current;
}


Image_data::Image_data(WriteableBitmap^ wb)
{
	wbmp = wb;
	nCol = wbmp->PixelWidth;
	nRow = wbmp->PixelHeight;
	size = nCol*nRow * 4;
}

Image_data::~Image_data()
{
}

Image_data::Image_data()
{
}

byte* Image_data::getPixelsPointer()
{
	if (pixels == nullptr)
	{
		IBuffer^ buffer = wbmp->PixelBuffer;

		// Query the IBufferByteAccess interface.
		ComPtr<IBufferByteAccess> bufferByteAccess;
		reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

		bufferByteAccess->Buffer(&pixels);
	}

	return pixels;
}

int Image_data::getSize()
{
	return size;
}

int Image_data::getnCol()
{
	return nCol;
}

int Image_data::getnRow()
{
	return nRow;
}



Platform::String^ stringToPlatformString(std::string inputString)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring intermediateForm = converter.from_bytes(inputString);
	Platform::String^ retVal = ref new Platform::String(intermediateForm.c_str());

	return retVal;
}

void LogMessage(Object^ parameter)
{
	// Function used to print informations in the output
	auto paraString = parameter->ToString();
	auto formattedText = std::wstring(paraString->Data()).append(L"\r\n");
	OutputDebugString(formattedText.c_str());
}


void Scenario3::file_from_picker(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	auto picker = ref new FileOpenPicker();
	picker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;
	picker->FileTypeFilter->Append(".bmp");
	picker->FileTypeFilter->Append(".jpg");
	picker->FileTypeFilter->Append(".jpeg");
	picker->FileTypeFilter->Append(".png");

	/* See FileAcces Samples for more information */
	auto pickingTask = Concurrency::create_task(picker->PickSingleFileAsync()); // User choose an image file
	pickingTask.then([this](StorageFile^ chosenfile)
	{
		this->pickedFile = chosenfile;
		return this->pickedFile->Properties->GetImagePropertiesAsync(); // Creation of a stream from the opened file
		
	}).then([this](FileProperties::ImageProperties^ prop)
	{
		int h = prop->Height;
		int w = prop->Width;
		this->WBimage = ref new WriteableBitmap(w,h); // (Columns, Rows)
		return this->pickedFile->OpenAsync(FileAccessMode(0)); // Sourcing of the WriteableBitmap object with the stream
		// Initialiser un objet Image_data
	}).then([this](IRandomAccessStream^ stream)
	{
		this->WBimage->SetSource(stream);
	}).then([this](void)
	{
		imData = Image_data(this->WBimage);
		std::stringstream st;
		st << imData.getnRow() << 'x' << imData.getnCol();
		std::string str = st.str();
		String^ St = stringToPlatformString(str);
		rootPage->NotifyUser("File open." + " Dimensions are " + St, NotifyType::StatusMessage);
	});
}



void Scenario3::SendImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StreamSocket^ socket = dynamic_cast<StreamSocket^>(CoreApplication::Properties->Lookup("clientSocket"));
	DataWriter^ writer;
	// If possible, use the DataWriter we created previously. If not, then create new one.
	if (!CoreApplication::Properties->HasKey("clientDataWriter"))
	{
		writer = ref new DataWriter(socket->OutputStream);
		//writer = ref new DataWriter();
		CoreApplication::Properties->Insert("clientDataWriter", writer);
	}
	else
	{
		writer = dynamic_cast<DataWriter^>(CoreApplication::Properties->Lookup("clientDataWriter"));
	}
	writer->UnicodeEncoding = UnicodeEncoding::Utf8;
	writer->ByteOrder = ByteOrder::BigEndian;

	// We first write image dimensions as two int32 and the 
	// the buffer.
	writer->WriteInt32(imData.getnRow());
	writer->WriteInt32(imData.getnCol());
	writer->WriteBuffer(this->WBimage->PixelBuffer);
	//writer->WriteBytes(Platform::ArrayReference<unsigned char>(&imData.getPixelsPointer()[0], imData.getSize()));

	// Sending of data
	create_task(writer->StoreAsync()).then([this, socket](task<unsigned int> writeTask)
	{
		try
		{
			// Try getting an exception.
			writeTask.get();
			SendOutput->Text = "Data sent successfully";
		}
		catch (Exception^ exception)
		{
			rootPage->NotifyUser("Send failed with error: " + exception->Message, NotifyType::ErrorMessage);
		}
	});
}

void Scenario3::Load_image_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{

	// Create a Uri object from the URI string
	// ms-appx:/// est la racine du projet
	Windows::Foundation::Uri^ uri = ref new Windows::Foundation::Uri("ms-appx:///Images/lena_color100.bmp");
	
	auto getfileTask = Concurrency::create_task(StorageFile::GetFileFromApplicationUriAsync(uri)); // User choose an image file
	getfileTask.then([this](StorageFile^ f)
	{
		this->pickedFile = f;
		return f->Properties->GetImagePropertiesAsync(); // Creation of a stream from the opened file

	}).then([this](FileProperties::ImageProperties^ prop)
	{
		int h = prop->Height;
		int w = prop->Width;
		this->WBimage = ref new WriteableBitmap(w, h); // (Columns, Rows)
		return this->pickedFile->OpenAsync(FileAccessMode(0)); // Sourcing of the WriteableBitmap object with the stream
															   // Initialiser un objet Image_data
	}) .then([this](IRandomAccessStream^ stream)
	{
		this->WBimage->SetSource(stream);
	}).then([this](void)
	{
		imData = Image_data(this->WBimage);
		std::stringstream st;
		st << imData.getnRow() << 'x' << imData.getnCol();
		std::string str = st.str();
		String^ St = stringToPlatformString(str);
		rootPage->NotifyUser("File open." + " Dimensions are " + St, NotifyType::StatusMessage);
	});

}
