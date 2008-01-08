/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkGDCMReader.h"

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkMedicalImageProperties.h"
#include "vtkStringArray.h"

#include "gdcmImageReader.h"
#include "gdcmDataElement.h"
#include "gdcmByteValue.h"
#include "gdcmSwapper.h"

#include <sstream>

vtkCxxRevisionMacro(vtkGDCMReader, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkGDCMReader);

//struct vtkGDCMReaderInternals
//{
//  gdcm::ImageReader DICOMReader;
//};

vtkGDCMReader::vtkGDCMReader()
{
  //this->Internals = new vtkGDCMReaderInternals;
  //this->ScalarArrayName = NULL;
  //this->SetScalarArrayName( "GDCM" );

  // vtkDataArray has an internal vtkLookupTable why not used it ?
  // vtkMedicalImageProperties is in the parent class
  //this->FileLowerLeft = 1;
}

vtkGDCMReader::~vtkGDCMReader()
{
  //delete this->Internals;
}

void vtkGDCMReader::ExecuteInformation()
{
  std::cerr << "ExecuteInformation" << std::endl;
}

void vtkGDCMReader::ExecuteData(vtkDataObject *output)
{
  std::cerr << "ExecuteData" << std::endl;
}

int vtkGDCMReader::CanReadFile(const char* fname)
{
  gdcm::ImageReader reader;
  reader.SetFileName( fname );
  if( reader.Read() )
    {
    return 0;
    }
  return 3;
}

//----------------------------------------------------------------------------
int vtkGDCMReader::ProcessRequest(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    return this->RequestData(request, inputVector, outputVector);
    }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    return this->RequestInformation(request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}


//
void vtkGDCMReader::FillMedicalImageInformation(const gdcm::ImageReader &reader)
{
  // For now only do:
  // PatientName, PatientID, PatientAge, PatientSex, PatientBirthDate,
  // StudyID
  std::ostringstream str;
  const gdcm::File &file = reader.GetFile();
  const gdcm::DataSet &ds = file.GetDataSet();

  const gdcm::Tag patname(0x0010, 0x0010);
  if( ds.FindDataElement( patname ) )
    {
    const gdcm::DataElement& de = ds.GetDataElement( patname );
    const gdcm::ByteValue *bv = de.GetByteValue();
    bv->Write<gdcm::SwapperNoOp>(str);
    assert( str.str().size() == bv->GetLength() );
    //std::string patname_str( bv->GetPointer(), bv->GetLength() );
    this->MedicalImageProperties->SetPatientName( str.str().c_str() );
    }
  str.str( "" );

/*
    {
    if (medprop->GetPatientName())
      {
      str.str("");
      str << medprop->GetPatientName();
      file->InsertValEntry(str.str(),0x0010,0x0010); // PN 1 Patient's Name
      }

    if (medprop->GetPatientID())
      {
      str.str("");
      str << medprop->GetPatientID();
      file->InsertValEntry(str.str(),0x0010,0x0020); // LO 1 Patient ID
      }

    if (medprop->GetPatientAge())
      {
      str.str("");
      str << medprop->GetPatientAge();
      file->InsertValEntry(str.str(),0x0010,0x1010); // AS 1 Patient's Age
      }

    if (medprop->GetPatientSex())
      {
      str.str("");
      str << medprop->GetPatientSex();
      file->InsertValEntry(str.str(),0x0010,0x0040); // CS 1 Patient's Sex
      }

    if (medprop->GetPatientBirthDate())
      {
      str.str("");
      str << medprop->GetPatientBirthDate();
      file->InsertValEntry(str.str(),0x0010,0x0030); // DA 1 Patient's Birth Date
      }

    if (medprop->GetStudyID())
      {
      str.str("");
      str << medprop->GetStudyID();
      file->InsertValEntry(str.str(),0x0020,0x0010); // SH 1 Study ID
      }
*/
}

//----------------------------------------------------------------------------
int vtkGDCMReader::RequestInformation(vtkInformation *request,
                                      vtkInformationVector **inputVector,
                                      vtkInformationVector *outputVector)
{
  // Let's read the first file :
  const char *filename;
  if( this->FileName )
    {
    filename = this->FileName;
    }
  else
    {
    assert( this->FileNames && this->FileNames->GetNumberOfValues() >= 1 );
    filename = this->FileNames->GetValue( 0 ).c_str();
    }
  gdcm::ImageReader reader;
  reader.SetFileName( filename );
  if( !reader.Read() )
    {
    return 0;
    }
  const gdcm::Image &image = reader.GetImage();
  const unsigned int *dims = image.GetDimensions();

  // Set the Extents.
  assert( image.GetNumberOfDimensions() >= 2 );
  this->DataExtent[0] = 0;
  this->DataExtent[1] = dims[0] - 1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = dims[1] - 1;
  if( image.GetNumberOfDimensions() == 2 )
    {
    // This is just so much painfull to deal with DICOM / VTK
    // they simply assume that number of file is equal to the dimension
    // of the last axe (see vtkImageReader2::SetFileNames )
    if ( this->FileNames && this->FileNames->GetNumberOfValues() > 1 )
      {
      this->DataExtent[4] = 0;
      //this->DataExtent[5] = this->FileNames->GetNumberOfValues() - 1;
      }
    else
      {
      this->DataExtent[4] = 0;
      this->DataExtent[5] = 0;
      }
    }
  else
    {
    assert( image.GetNumberOfDimensions() == 3 );
    this->FileDimensionality = 3;
    this->DataExtent[4] = 0;
    this->DataExtent[5] = dims[2] - 1;
    }
  //this->DataSpacing[0] = 1.;
  //this->DataSpacing[1] = -1.;
  //this->DataSpacing[2] = 1.;

  gdcm::PixelType pixeltype = image.GetPixelType();
  switch( pixeltype )
    {
  case gdcm::PixelType::INT8:
    this->DataScalarType = VTK_SIGNED_CHAR;
    break;
  case gdcm::PixelType::UINT8:
    this->DataScalarType = VTK_UNSIGNED_CHAR;
    break;
  case gdcm::PixelType::INT12:
    abort();
    this->DataScalarType = VTK_SHORT;
    break;
  case gdcm::PixelType::UINT12:
    abort();
    this->DataScalarType = VTK_UNSIGNED_SHORT;
    break;
  case gdcm::PixelType::INT16:
    this->DataScalarType = VTK_SHORT;
    break;
  case gdcm::PixelType::UINT16:
    this->DataScalarType = VTK_UNSIGNED_SHORT;
    break;
  default:
    ;
    }

  this->NumberOfScalarComponents = pixeltype.GetSamplesPerPixel();
  if( image.GetPhotometricInterpretation() == 
    gdcm::PhotometricInterpretation::PALETTE_COLOR )
    {
    assert( this->NumberOfScalarComponents == 1 );
    this->NumberOfScalarComponents = 3;
    }

  int numvol = 1;
  this->SetNumberOfOutputPorts(numvol);
  // For each output:
  for(int i = 0; i < numvol; ++i)
    {
    // Allocate !
    if( !this->GetOutput(i) )
      {
      vtkImageData *img = vtkImageData::New();
      this->GetExecutive()->SetOutputData(i, img );
      img->Delete();
      }
    vtkInformation *outInfo = outputVector->GetInformationObject(i);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
      0, DataExtent[1], 0, DataExtent[3], 0, DataExtent[5]);

    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->DataScalarType, this->NumberOfScalarComponents);
    //outInfo->Set(vtkDataObject::SPACING(), spcs, 3);

    double origin[3] = {};
    outInfo->Set(vtkDataObject::ORIGIN(), origin, 3);
    }

  // Ok let's fill in the 'extra' info:
  FillMedicalImageInformation(reader);

//  return this->Superclass::RequestInformation(
//    request, inputVector, outputVector);
  return 1;
}

int LoadSingleFile(const char *filename, int *dext, char *pointer)
{
  gdcm::ImageReader reader;
  reader.SetFileName( filename );
  if( !reader.Read() )
    {
    return 0;
    }

  const gdcm::Image &image = reader.GetImage();
  unsigned long len = image.GetBufferLength();
  char *tempimage = new char[len];
  image.GetBuffer(tempimage);

  const unsigned int *dims = image.GetDimensions();
  gdcm::PixelType pixeltype = image.GetPixelType();
  long outsize = pixeltype.GetPixelSize()*(dext[1] - dext[0] + 1);
  //std::cerr << "dext: " << dext[2] << " " << dext[3] << std::endl;
  //std::cerr << "dext: " << dext[4] << " " << dext[5] << std::endl;
  //memcpy(pointer, tempimage, len);
  for(int j = dext[4]; j <= dext[5]; ++j)
  {
    //std::cerr << j << std::endl;
    for(int i = dext[2]; i <= dext[3]; ++i)
      {
      //memcpy(pointer, tempimage+i*outsize, outsize);
      //memcpy(pointer, tempimage+(this->DataExtent[3] - i)*outsize, outsize);
      //memcpy(pointer, tempimage+(i+j*(dext[3]+1))*outsize, outsize);
      memcpy(pointer,
        tempimage+((dext[3] - i)+j*(dext[3]+1))*outsize, outsize);
      pointer += outsize;
      }
  }
  delete[] tempimage;

  return 1; // success
}

//----------------------------------------------------------------------------
int vtkGDCMReader::RequestData(vtkInformation *vtkNotUsed(request),
                                vtkInformationVector **vtkNotUsed(inputVector),
                                vtkInformationVector *outputVector)
{

  //this->UpdateProgress(0.2);

  // Make sure the output dimension is OK, and allocate its scalars

  for(int i = 0; i < this->GetNumberOfOutputPorts(); ++i)
  {
  // Copy/paste from vtkImageAlgorithm::AllocateScalars. Cf. "this needs to be fixed -Ken"
    vtkStreamingDemandDrivenPipeline *sddp = 
      vtkStreamingDemandDrivenPipeline::SafeDownCast(this->GetExecutive());
    if (sddp)
      {
      int extent[6];
      sddp->GetOutputInformation(i)->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),extent);
      this->GetOutput(i)->SetExtent(extent);
      }
    this->GetOutput(i)->AllocateScalars();
  }


//  vtkInformation *outInfo = outputVector->GetInformationObject(0);
//  vtkImageData *output = vtkImageData::SafeDownCast(
//    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  int *dext = this->GetDataExtent();
//  output->SetDimensions(
//    dext[1] - dext[0] + 1, dext[3] - dext[2] + 1, dext[5] - dext[4] + 1);
//  output->AllocateScalars();

  vtkImageData *output = this->GetOutput(0);
  char * pointer = static_cast<char*>(output->GetScalarPointer());
  if( this->FileName )
    {
    const char *filename = this->FileName;
    LoadSingleFile( filename, dext, pointer );
    return 1;
    }
  else
    {
    assert( this->FileNames && this->FileNames->GetNumberOfValues() >= 1 );
    }

  // Load each 2D files
  for(int j = dext[4]; j <= dext[5]; ++j)
    {
    gdcm::ImageReader reader;
    const char *filename;
    filename = this->FileNames->GetValue( j ).c_str();
    //std::cerr << "Reader:" << j << " -> " << filename << std::endl;
    reader.SetFileName( filename );
    if( !reader.Read() )
      {
      // TODO need to do some cleanup...
      return 0;
      }

    const gdcm::Image &image = reader.GetImage();
    unsigned long len = image.GetBufferLength();
    char *tempimage = new char[len];
    image.GetBuffer(tempimage);

    const unsigned int *dims = image.GetDimensions();
    gdcm::PixelType pixeltype = image.GetPixelType();
    long outsize = pixeltype.GetPixelSize()*(dext[1] - dext[0] + 1);
    //std::cerr << "dext: " << dext[2] << " " << dext[3] << std::endl;
    //std::cerr << "dext: " << dext[4] << " " << dext[5] << std::endl;
#if 1
    memcpy(pointer, tempimage, len);
    pointer += len;
#else
    //std::cerr << j << std::endl;
    for(int i = dext[2]; i <= dext[3]; ++i)
      {
      //memcpy(pointer, tempimage+i*outsize, outsize);
      //memcpy(pointer, tempimage+(this->DataExtent[3] - i)*outsize, outsize);
      //memcpy(pointer, tempimage+(i+j*(dext[3]+1))*outsize, outsize);
      memcpy(pointer,
        tempimage+((dext[3] - i)+j*(dext[3]+1))*outsize, outsize);
      pointer += outsize;
      }
#endif
    delete[] tempimage;
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkGDCMReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

