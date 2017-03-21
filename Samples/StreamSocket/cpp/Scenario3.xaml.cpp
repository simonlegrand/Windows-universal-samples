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
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
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

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw Exception::CreateException(hr);
	}
}


Image_data::Image_data(WriteableBitmap^ wbmp)
{
	IBuffer^ buffer = wbmp->PixelBuffer;
	size = buffer->Length;
	// Query the IBufferByteAccess interface.
	ComPtr<IBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	bufferByteAccess->Buffer(&pixels);

	nCol = wbmp->PixelWidth;
	nRow = wbmp->PixelHeight;

	/*for (int y = 0; y < nRow; y++)
	{
		for (int x = 0; x < nCol; x++)
		{
			pixels[(x + y * nCol) * 4] = 15; // B
			pixels[(x + y * nCol) * 4 + 1] = 15; // G
			pixels[(x + y * nCol) * 4 + 2] = 15; // R
			pixels[(x + y * nCol) * 4 + 3] = 15; // A
		}
	}*/
}

Image_data::~Image_data()
{
}

Image_data::Image_data()
{
}

byte* Image_data::getPixelsPointer()
{
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
		//rootPage->NotifyUser("The file '" + this->pickedFile->Name + "' was found.", NotifyType::StatusMessage);
	}).then([this](void)
	{
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
		rootPage->NotifyUser("File open." + " Dimesions are " + St, NotifyType::StatusMessage);
	});
}


byte* GetPointerToPixelData(IBuffer^ buffer)
{
	// Cast to Object^, then to its underlying IInspectable interface.
	Object^ obj = buffer;
	ComPtr<IInspectable> insp(reinterpret_cast<IInspectable*>(obj));

	// Query the IBufferByteAccess interface.
	ComPtr<IBufferByteAccess> bufferByteAccess;
	ThrowIfFailed(insp.As(&bufferByteAccess));

	// Retrieve the buffer data.
	byte* pixels = nullptr;
	ThrowIfFailed(bufferByteAccess->Buffer(&pixels));
	return pixels;
}

/* 
	Returns a pointer on the pixels from the buffer and add bytes at the beginning
	containing the number of pixels on each dimension.
*/
byte* Scenario3::getDataAndHeader(IBuffer^ buf)
{
	int size_data = buf->Length;
	// See here for more info:
	// http://www.davidbritch.com/2012/09/accessing-image-pixel-data-in-ccx.html
	byte *data = GetPointerToPixelData(buf);
	int w = this->WBimage->PixelWidth;
	int h = this->WBimage->PixelHeight;
	
	// Container for the data.
	// We add 8 extra bytes at the beginning of header_data
	// that conatins image dimension (in pixels)
	byte* header_data = new byte [8 + size_data];
	
	intToBytes(h, header_data);
	intToBytes(w, header_data + 4);
	for (int i = 0; i < size_data; i++)
		header_data[i + 8] = data[i];

	/*std::stringstream st;
	st << (int8)header_data[0] << '-' << (int8)header_data[1] << '-' << (int8)header_data[2] << '-' << (int8)header_data[3];
	std::string height = st.str();
	String^ Str = stringToPlatformString(height);
	LogMessage(Str);*/
	
	return header_data;
}

void Scenario3::intToBytes(int value, byte* b)
{
	/* int are four bytes long */
	b[0] = value >> 24;
	b[1] = value >> 16;
	b[2] = value >> 8;
	b[3] = value;
}


void Scenario3::SendImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	/* Send image data to the server, as a string */
	//byte* pix = getDataAndHeader(this->WBimage->PixelBuffer);
	
	/* This part can be used to send a string */
	//std::stringstream st;
	//int height = this->image->PixelHeight;
	//int width = this->image->PixelWidth;
	//st << height << 'x' << width;
	//st << '-';
	//for (uint8 i = 0; i < 200; i++)
	//{
	//	st << pix[i] << ",";
	//}
	//st << '\n'; // Line terminator for the server.
	//std::string str = st.str();
	//String^ Size = this->image->PixelBuffer->Length.ToString();
	//String^ Str = stringToPlatformString(str);
	//LogMessage(Size);
	//LogMessage(Str);


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

	//writer->WriteString(Str);
	//writer->WriteBuffer(PixBuf);
	writer->WriteInt32(imData.getnRow());
	writer->WriteInt32(imData.getnCol());
	writer->WriteBytes(ArrayReference<unsigned char>(&imData.getPixelsPointer()[0], imData.getSize()));

	std::stringstream ssr;
	ssr << writer->UnstoredBufferLength;
	std::string reste = ssr.str();
	LogMessage(stringToPlatformString(reste));
	create_task(writer->StoreAsync()).then([this, socket](task<unsigned int> writeTask)
	{
		try
		{
			// Try getting an exception.
			writeTask.get();
			SendOutput->Text = "data sent successfully";
		}
		catch (Exception^ exception)
		{
			rootPage->NotifyUser("Send failed with error: " + exception->Message, NotifyType::ErrorMessage);
		}
	});
}


void Scenario3::SendHello_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (!CoreApplication::Properties->HasKey("connected"))
    {
        rootPage->NotifyUser("Please run previous steps before doing this one.", NotifyType::ErrorMessage);
        return;
    }

    StreamSocket^ socket = dynamic_cast<StreamSocket^>(CoreApplication::Properties->Lookup("clientSocket"));
    DataWriter^ writer;

    // If possible, use the DataWriter we created previously. If not, then create new one.
    if (!CoreApplication::Properties->HasKey("clientDataWriter"))
    {
        writer = ref new DataWriter(socket->OutputStream);
        CoreApplication::Properties->Insert("clientDataWriter", writer);
    }
    else
    {
        writer = dynamic_cast<DataWriter^>(CoreApplication::Properties->Lookup("clientDataWriter"));
    }
	writer->UnicodeEncoding = UnicodeEncoding::Utf8;
	//writer->ByteOrder = ByteOrder::BigEndian;
   
	//std::string filename = "C:\\Users\\slegrand\\Documents\\Windows-universal-samples\\Samples\\StreamSocket\\cpp\\lemmy.bmp";
	////std::string filename = "lemmy.bmp";
	//cv::Mat E = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	//if (!E.data)                              // Check for invalid input
	//{
	//	LogMessage("Could not open or find the image");
	//}
	////cv::Mat E;
	////cv::cvtColor(image, E, CV_16UC1);

	cv::Mat E = cv::Mat::eye(20, 20, CV_8UC1);
	//cv::Mat E = (cv::Mat_<int>(3, 3) << 1, 0, 4, 0, 1, 0, 0, 0, 1);
	uint8_t *myData = E.data;
	uint width = E.cols;
	uint height = E.rows;
	std::stringstream ss;


	/* Voir si on peut pas creer un WritableBitmap qu'on remplit avec */

	//Tranform each element of Mat object into array of string objects
	ss << height << 'x' << width;
	ss << '-'; //Separation character between matrix size and data.

	for (uint i = 0; i < height; i++)
	{
		for (uint j = 0; j < width; j++)
		{
			ss << (int)myData[i * width + j] << ",";
		}
	}
	ss << '\n'; // Line terminator for the server.
	std::string str = ss.str();
	String^ Str = stringToPlatformString(str);
	
	unsigned int inputElementSize = writer->MeasureString(Str);
	writer->WriteString(Str);
	LogMessage(Str);
	LogMessage(Str->Length());
	
    String^ stringToSend("Ca marche!");

	// **************************************
	// Here we can try to create get a frame object from the camera and send it to the server.
	// See how interface this code and HoloFaceTracking.
	// **************************************

    // Write the locally buffered data to the network.
    create_task(writer->StoreAsync()).then([this, socket] (task<unsigned int> writeTask)
    {
        try
        {
            // Try getting an exception.
            writeTask.get();
            SendOutput->Text = "data sent successfully";
        }
        catch (Exception^ exception)
        {
            rootPage->NotifyUser("Send failed with error: " + exception->Message, NotifyType::ErrorMessage);
        }
    });

	// Just to check wether the buffer is empty or not
	std::stringstream ssr;
	ssr << writer->UnstoredBufferLength;
	std::string reste = ssr.str();
	LogMessage(stringToPlatformString(reste));
}


