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
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

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
    // Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
    // the data locally.
	//cv::Mat image = cv::imread('l-samples\Samples\StreamSocket\cpp\Images', CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat E;
	//cv::cvtColor(image, E, CV_16UC1);
	String^ test("pouet");
	LogMessage(test);
	//cv::Mat E = cv::Mat::eye(20, 20, CV_8UC1);
	cv::Mat E = (cv::Mat_<int>(3, 3) << 1, 0, 4, 0, 1, 0, 0, 0, 1);
	uint8_t *myData = E.data;
	uint width = E.cols;
	uint height = E.rows;
	std::stringstream ss;

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
	std::string str = ss.str();
	String^ Str = stringToPlatformString(str);
	//String^ Str_bis(str.c_str);
	
	
	unsigned int inputElementSize = writer->MeasureString(Str);
	writer->WriteString(Str);
	LogMessage(Str);
	LogMessage(Str->Length());
	
    String^ stringToSend("Ca marche!");

	std::stringstream ssr;
	ssr << writer->UnstoredBufferLength;
	std::string reste = ssr.str();
	LogMessage(stringToPlatformString(reste));
	// **************************************
	// Here we can try to create get a frame object from the camera and send it to the server.
	// See how interface this code and HoloFaceTracking.
	// **************************************

    // Write the locally buffered data to the network.
	DataWriterStoreOperation^ ope = writer->StoreAsync();
	AsyncStatus status = ope->Status;
	LogMessage(status);
    //create_task(writer->StoreAsync()).then([this, socket, stringToSend] (task<unsigned int> writeTask)
    //{
    //    try
    //    {
    //        // Try getting an exception.
    //        writeTask.get();
    //        SendOutput->Text = "\"" + stringToSend + "\" sent successfully";
    //    }
    //    catch (Exception^ exception)
    //    {
    //        rootPage->NotifyUser("Send failed with error: " + exception->Message, NotifyType::ErrorMessage);
    //    }
    //});

	/*std::stringstream ssr;
	ssr << writer->UnstoredBufferLength;
	std::string reste = ssr.str();
	LogMessage(stringToPlatformString(reste));*/
}
