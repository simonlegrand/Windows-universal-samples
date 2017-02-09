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

    // Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
    // the data locally.
	//cv::Mat image = cv::imread(".\Images\lemmy1.jpg");
	cv::Mat E = cv::Mat::eye(4, 4, CV_64F);
	//std::string s = std::to_string(E.data);
	//String^ S = stringToPlatformString(s);

	LogMessage("coucou");
	//cv::imshow("Display window", image);

    String^ stringToSend("Ca marche!");
    writer->WriteUInt32(writer->MeasureString(stringToSend));
    writer->WriteString(stringToSend);

	// **************************************
	// Here we can try to create get a frame object from the camera and send it to the server.
	// See how interface this code and HoloFaceTracking.
	// **************************************


    // Write the locally buffered data to the network.
    create_task(writer->StoreAsync()).then([this, socket, stringToSend] (task<unsigned int> writeTask)
    {
        try
        {
            // Try getting an exception.
            writeTask.get();
            SendOutput->Text = "\"" + stringToSend + "\" sent successfully";
        }
        catch (Exception^ exception)
        {
            rootPage->NotifyUser("Send failed with error: " + exception->Message, NotifyType::ErrorMessage);
        }
    });
}
