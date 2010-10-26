/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2010 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmStreamImageReader.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmSystem.h"
#include "gdcmFilename.h"
#include "gdcmByteSwap.h"
#include "gdcmTrace.h"
#include "gdcmTesting.h"

int TestStreamImageRead(const char* filename, bool verbose = false, bool lossydump = false)
{
  if( verbose )
    std::cerr << "Reading: " << filename << std::endl;
  gdcm::StreamImageReader reader;

  reader.SetFileName( filename );
  if ( reader.ReadImageInformation() )
    {
    int res = 0;
    //std::cerr << "Success to read image from file: " << filename << std::endl;
    unsigned long len = reader.DefineProperBufferLength();

    char* buffer = new char[len];
    bool res2 = reader.Read(buffer, len);
    if( !res2 )
      {
      std::cerr << "res2 failure:" << filename << std::endl;
      return 1;
      }
    // On big Endian system we have byteswapped the buffer (duh!)
    // Since the md5sum is byte based there is now way it would detect
    // that the file is written in big endian word, so comparing against
    // a md5sum computed on LittleEndian would fail. Thus we need to
    // byteswap (again!) here:
#ifdef GDCM_WORDS_BIGENDIAN
    if( img.GetPixelFormat().GetBitsAllocated() == 16 )
      {
      assert( !(len % 2) );
      // gdcm-US-ALOKA is a 16 bits image with PALETTE
      //assert( img.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::MONOCHROME1
      //  || img.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::MONOCHROME2 );
      gdcm::ByteSwap<unsigned short>::SwapRangeFromSwapCodeIntoSystem(
        (unsigned short*)buffer, gdcm::SwapCode::LittleEndian, len/2);
      }
#endif
    const char *ref = gdcm::Testing::GetMD5FromFile(filename);

    char digest[33];
    gdcm::Testing::ComputeMD5(buffer, len, digest);
    if( verbose )
      {
      std::cout << "ref=" << ref << std::endl;
      std::cout << "md5=" << digest << std::endl;
      }
    if( !ref )
      {
      // new regression image needs a md5 sum
      std::cout << "Missing md5 " << digest << " for: " << filename <<  std::endl;
      //assert(0);
      res = 1;
      }
    else if( strcmp(digest, ref) )
      {
      std::cerr << "Problem reading image from: " << filename << std::endl;
      std::cerr << "Found " << digest << " instead of " << ref << std::endl;
      res = 1;
#if 0
      std::ofstream debug("/tmp/dump.gray");
      debug.write(buffer, len);
      debug.close();
      //assert(0);
#endif
      }
    delete[] buffer;
    return res;
    }

  const gdcm::FileMetaInformation &header = reader.GetFile().GetHeader();
  gdcm::MediaStorage ms = header.GetMediaStorage();
  bool isImage = gdcm::MediaStorage::IsImage( ms );
  if( isImage )
    {
    if( reader.GetFile().GetDataSet().FindDataElement( gdcm::Tag(0x7fe0,0x0010) ) )
      {
      std::cerr << "Failed to read image from file: " << filename << std::endl;
      return 1;
      }
    else
      {
      std::cerr << "no Pixel Data Element found in the file:" << filename << std::endl;
      return 0;
      }
    }
  // else
  // well this is not an image, so thankfully we fail to read it
  std::cerr << "Could not read image(" << filename << "), since file is a: " << ms << std::endl;
  //assert( ms != gdcm::MediaStorage::MS_END );
  return 0;
}

int TestStreamImageReader(int argc, char *argv[])
{
  if( argc == 2 )
    {
    const char *filename = argv[1];
    return TestStreamImageRead(filename, true);
    }

  // else
  // First of get rid of warning/debug message
  gdcm::Trace::DebugOff();
  gdcm::Trace::WarningOff();
  int r = 0, i = 0;
  const char *filename;
  const char * const *filenames = gdcm::Testing::GetFileNames();
  while( (filename = filenames[i]) )
    {
    r += TestStreamImageRead( filename);
    ++i;
    }

  return r;
}
