/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

using System;
using gdcm;

public class HelloWorld
{
  public static void Main(string[] args)
    {
    System.Console.WriteLine("Hello World !");
    //gdcm.Reader reader2;
    string filename = args[0];
    System.Console.WriteLine( "Reading: " + filename );
    Reader reader = new Reader();
    reader.SetFileName( filename );
    bool ret = reader.Read();
    if( !ret )
      {
      throw new Exception("Could not read: " + filename );
      }
    //std::cout << reader.GetFile() 
    Tag t = new Tag(0x21,0x12);
    System.Console.WriteLine( "out:" + t.toString() );
    System.Console.WriteLine( "out:" + reader.GetFile().GetDataSet().toString() );
    }
}
