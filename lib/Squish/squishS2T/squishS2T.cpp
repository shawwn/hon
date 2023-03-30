#include <squish.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <IL/ilu.h>

typedef unsigned int	uint;
typedef unsigned char	byte;

// From the K2 Engine, for consistency's sake
int	NextPowerOfTwo(int x)
{
	// Check that it isn't already a power of two
	for (int n = 0; n < 32; ++n)
	{
		if (x == (1 << n))
			return x;
	}

	// Find the highest bit
	int c = 0;
	while (x)
	{
		x >>= 1;
		++c;
	}
	
	return (1 << c);
}

FILE *f, *fC1, *fC2, *fW, *fA;
bool ProcessImage(ILubyte* data, ILuint Width, ILuint Height, ILuint Bpp, bool highQuality)
{
	int	 flags;
	bool useAlpha = (Bpp >= 4);
	uint numBlocksX = (int)((Width/4));
	uint numBlocksY = (int)((Height/4));

	// Check for alpha
	// If there's no used alpha, use Opaque DXT1
	// If there's alpha, always use DXT5 since Transparent DXT1 is lower quality
	if(useAlpha)
	{
		useAlpha = false;
		for(uint y = 0; y < Height; y += 1)
		{
			for(uint x = 0; x < Width; x += 1)
			{
				if(data[((x+(y*Width))*Bpp)+3] < 255)
				{
					useAlpha = true;
					break;
				}
			}
		}
		if(useAlpha)
			flags = squish::kDxt5;
		else
			flags = squish::kDxt1;
	}
	else
		flags = squish::kDxt1;

	// Allocate memory and loop over the blocks
	{
		uint blocknum = 0;
		uint allocationWidth  = (int)((float)(Width+3)/4.0f);
		uint allocationHeight = (int)((float)(Height+3)/4.0f);
		uint allocationSize = allocationWidth*allocationHeight*8;

		byte* colors1 = (byte*)malloc(allocationSize/4);
		byte* colors2 = (byte*)malloc(allocationSize/4);
		byte* weights = (byte*)malloc(allocationSize/2);
		byte* alpha = (useAlpha) ? (byte*)malloc(allocationSize) : NULL;

		// Loop over blocks
		for(uint y = 0; y < Height; y += 4 )
		{
			for(uint x = 0; x < Width; x += 4 )
			{
				// build the 4x4 block of pixels
				byte sourceRgba[16*4];
				byte* targetPixel = sourceRgba;
				byte block[16];
				int mask = 0;
				for( uint py = 0; py < 4; ++py )
				{
					for( uint px = 0; px < 4; ++px )
					{
						// get the source pixel in the image
						uint sx = x + px;
						uint sy = y + py;
							
						// enable if we're in the image
						if( sx < Width && sy < Height )
						{
							// copy the rgba value
							byte const* sourcePixel = data + 4*( Width*sy + sx );
							for( int i = 0; i < 4; ++i )
								*targetPixel++ = *sourcePixel++;
								
							// enable this pixel
							mask |= ( 1 << ( 4*py + px ) );
						}
						else
						{
							// skip this pixel as its outside the image
							targetPixel += 4;
						}
					}
				}
				
				// Compress it into the output
				squish::CompressMasked( sourceRgba, mask, block, highQuality ? (flags|squish::kColourIterativeClusterFit) : (flags|squish::kColourRangeFit));

				colors1[blocknum*2+0] = block[0];
				colors1[blocknum*2+1] = block[1];
				colors2[blocknum*2+0] = block[2];
				colors2[blocknum*2+1] = block[3];

				// Pack it better for the compressor
				weights[blocknum*4+0] = (block[4] & (1+2+4+8))|((block[5] & (1+2+4+8))<<4);
				weights[blocknum*4+1] = (block[6] & (1+2+4+8))|((block[7] & (1+2+4+8))<<4);
				weights[blocknum*4+2] = ((block[4] & (16+32+64+128))>>4)|((block[5] & (16+32+64+128)));
				weights[blocknum*4+3] = ((block[6] & (16+32+64+128))>>4)|((block[7] & (16+32+64+128)));

				if(alpha)
				{
					alpha[blocknum*8+0] = block[8];
					alpha[blocknum*8+1] = block[9];
					alpha[blocknum*8+2] = block[10];
					alpha[blocknum*8+3] = block[11];
					alpha[blocknum*8+4] = block[12];
					alpha[blocknum*8+5] = block[13];
					alpha[blocknum*8+6] = block[14];
					alpha[blocknum*8+7] = block[15];
				}

				blocknum++;
			}
		}
		// Write the data to the files, we'll read it back later
		fwrite(colors1, allocationSize/4, 1, fC1);
		fwrite(colors2, allocationSize/4, 1, fC2);
		fwrite(weights, allocationSize/2, 1, fW);
		if(alpha)
			fwrite(alpha, allocationSize, 1, fA);

		// Free the temporary buffers...
		free(colors1);
		free(colors2);
		free(weights);
		if(alpha)
			free(alpha);
	}
	return true;
}

int main(int argc, char* argv[])
{
	bool highQuality = false;
	char Filename[4096] = {0};
	char Destname[4096] = {0};
	if(argc < 3)
	{
		printf("Please pass input and output filenames!\n");
		return -1;
	}

	strcpy(Filename, argv[1]);
	strcpy(Destname, argv[2]);
	if(argc >= 4 && stricmp(argv[3], "hq") == 0)
		highQuality = true;

	// Load the image...
	ilInit();
	ILenum Error;
	ILuint ImageName;
	ilGenImages(1, &ImageName);
	ilBindImage(ImageName);
	ilLoadImage(Filename);

	// Check for errors...
	bool hadError = false;
	while ((Error = ilGetError()) != IL_NO_ERROR)
	{
		printf("%d: %s\n", Error, iluErrorString(Error));
		hadError = true;
	}
	if(hadError)
		return -1;

	// Get the data...
	ILuint Width, Height, Bpp;
	Bpp = ilGetInteger(IL_IMAGE_BPP);
	Width = NextPowerOfTwo(ilGetInteger(IL_IMAGE_WIDTH));
	Height = NextPowerOfTwo(ilGetInteger(IL_IMAGE_HEIGHT));

	// Make sure the file is not a complete joke!
	if(Width < 1 || Height < 1 || Bpp < 1)
		return -1;

	iluImageParameter(ILU_FILTER, ILU_BILINEAR);
	ILubyte *Data = (ILubyte*)malloc(Width*Height*4);

	if(Width != ilGetInteger(IL_IMAGE_WIDTH) || Height != ilGetInteger(IL_IMAGE_HEIGHT))
		iluScale(Width, Height, 1);

	// DXTC works on 4x4 blocks... So the largest mipmap should at least be 4x4!
	if(Width < 4 || Height < 4)
		iluScale((Width < 4) ? 4 : Width, (Height < 4) ? 4 : Height, 1);

	// Open the temporary files
	f = fopen(Destname, "wb");
	fC1 = fopen("C1-TEMP", "w+b");
	fC2 = fopen("C2-TEMP", "w+b");
	fW  = fopen("W-TEMP", "w+b");
	fA  = fopen("A-TEMP", "w+b");

	// Initialize the width/height and alpha variables
	short w = Width;
	short h = Height;
	byte v = (Bpp >= 4);
	fwrite(&v, 1, 1, f);

	// Write the various mipmaps to the temporary files
	while(1)
	{
		// Header (real file)
		fwrite(&w, 2, 1, f);
		fwrite(&h, 2, 1, f);
		// Data
		ilCopyPixels(0, 0, 0, w, h, 1, IL_RGBA, IL_UNSIGNED_BYTE, Data);
		ProcessImage(Data, w, h, Bpp, highQuality);
		// Stop?
		if(w == 1 && h == 1)
			break;
		// Scale
		w /= 2; if(w < 1) w = 1;
		h /= 2; if(h < 1) h = 1;
		iluScale(w, h, 1);
	}
	// How much data did we write to the temporary files?
	int sizeC1 = ftell(fC1);
	int sizeC2 = ftell(fC2);
	int sizeW = ftell(fW);
	int sizeA = ftell(fA);

	// Reset the position in the temp files so we can read it back
	fseek(fC1, 0, SEEK_SET);
	fseek(fC2, 0, SEEK_SET);
	fseek(fW, 0, SEEK_SET);
	fseek(fA, 0, SEEK_SET);

	// Read the data back and write it 
	fread(Data, sizeC1, 1, fC1);
	fwrite(Data, sizeC1, 1, f);
	fread(Data, sizeC2, 1, fC2);
	fwrite(Data, sizeC2, 1, f);
	fread(Data, sizeW, 1, fW);
	fwrite(Data, sizeW, 1, f);
	fread(Data, sizeA, 1, fA);
	fwrite(Data, sizeA, 1, f);

	// Close the files
	fclose(fC1);
	fclose(fC2);
	fclose(fW);
	fclose(fA);
	fclose(f);

	// And delete the temporary files, too...
	remove("C1-TEMP");
	remove("C2-TEMP");
	remove("W-TEMP");
	remove("A-TEMP");

	// Check for errors...
	hadError = false;
	while ((Error = ilGetError()) != IL_NO_ERROR)
	{
		printf("%d: %s\n", Error, iluErrorString(Error));
		hadError = true;
	}
	if(hadError)
	{
		remove(Destname);
		return -1;
	}

	// Free the image...
	ilDeleteImages(1, &ImageName);
	free(Data);

	return 0;
}
