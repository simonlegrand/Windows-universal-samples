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
// Scenario3.xaml.h
// Declaration of the Scenario3 class
//

#pragma once

#include "pch.h"
#include "Scenario3.g.h"
#include "MainPage.xaml.h"

namespace SDKTemplate
{
    namespace StreamSocketSample
    {
        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        

		class Image_data
		{
		private:
			int nCol = 0;
			int nRow = 0;
			unsigned int size = 0; //Size of pixels array in bytes
			Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ wbmp;
			byte* pixels = nullptr;

		public:
			Image_data();
			Image_data(Windows::UI::Xaml::Media::Imaging::WriteableBitmap^);
			~Image_data();
			uint8* getPixelsPointer();
			int getSize();
			int getnCol();
			int getnRow();
		};

		[Windows::Foundation::Metadata::WebHostHidden]
        public ref class Scenario3 sealed
        {
        public:
            Scenario3();

        protected:
            virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        private:
            MainPage^ rootPage;
			Windows::Storage::StorageFile^ pickedFile;
			Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ WBimage;
			Image_data imData;

			void file_from_picker(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
			void SendImage_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        };
    }
}
