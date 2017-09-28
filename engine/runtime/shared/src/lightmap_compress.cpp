
#include "bdefs.h"
#include "lightmap_compress.h"
#include "lightmapdefs.h"


//outputs a span into a byte array, and updates the pointer accordingly
static void OutputSpan(bool bRun, uint32 nSpanLen, uint8* pInData, uint8* &pOutData)
{
	//sanity checks
	ASSERT(pInData);
	ASSERT(pOutData);
	ASSERT(nSpanLen <= 128);
	ASSERT(nSpanLen > 0);

	//output the tag. The high bit indicates a run, the other 7 bits indicate the
	//span length (-1)
	uint8 nTag = (bRun) ? 0x80 : 0x00;
	nTag |= (uint8)(nSpanLen - 1);

	*pOutData = nTag;
	pOutData++;

	//output the span...only a single color if it is a run
	if(bRun)
	{
		for(uint32 nChannel = 0; nChannel < 3; nChannel++)
		{
			*pOutData = pInData[nChannel];
			pOutData++;
		}
	}
	else
	{
		//output every color in the span
		for(uint32 nColor = 0; nColor < nSpanLen; nColor++)
		{
			for(uint32 nChannel = 0; nChannel < 3; nChannel++)
			{
				*pOutData = pInData[nColor * 3 + nChannel];
				pOutData++;
			}
		}
	}

}


//compresses the 24 bit lightmap data found in pData into the output buffer pOut. 
//Returns the sucess code
//
//the compression algorithm is of the form of TGA files, where a tag byte
//indicates if it is a run or not with the high bit, and then uses the remaining
//seven bits to encode the length. If it is not a run, raw data follows for the
//specified span.
//
// pOut MUST be (LIGHTMAP_MAX_DATA_SIZE) in length.
// outLen is filled in with how much data is actually needed in pOut.
// outLen is guaranteed to be less than LIGHTMAP_MAX_DATASIZE (and usually less than
// or equal to width * height * 3).
bool CompressLMData(uint8 *pData, 
	uint32 width, uint32 height, 
	uint8 *pOutBuffer, uint32 &outLen)
{
    uint32 nCurrPel;
    
	//first off, make sure that we have valid buffers
	if ((pData == NULL) || (pOutBuffer == NULL)) {
		assert(FALSE); return FALSE; }				//invalid buffers

	//make sure that the buffer will accomodate the lightmap page based
	//upon the assumptions made above in the function spec
	if ((width > LIGHTMAP_MAX_PIXELS_I) || (height > LIGHTMAP_MAX_PIXELS_I)) {
		assert(FALSE); return FALSE; }				//invalid lightmap size

	//number of bytes in the input buffer
	uint32 nBufferLen = width * height * 3;

	//the span, whether it be run, or raw data
	uint32 nSpanLen = 0;

	//the output cursor
	uint8* pOutPos = pOutBuffer;

	//run through the input buffer
	for ( nCurrPel = 0; nCurrPel < nBufferLen; nCurrPel += 3)
    {
		uint32 nRunLen = 1;

		//check and see if we are starting a run
		for (uint32 nRunPel = nCurrPel + 1; nRunPel < nBufferLen; nRunPel += 3, nRunLen++)
        {
			//need to make sure that the run length is still in range
			if (nRunLen > 127)
            {
                break;
            }

			//check to see if the color is the same (note, this could use some
			//sort of luminance delta in order to acheive better compression,
			//but for now, lossless should be done and then it can be
			//experimented with)
			if( (pData[nCurrPel + 0] != pData[nRunPel + 0]) || (pData[nCurrPel + 1] != pData[nRunPel + 1]) || (pData[nCurrPel + 2] != pData[nRunPel + 2]) ) {
				break; } }								//different color, bust out

		//see if we hit a run
		if(nRunLen >= 2)
		{
			//we hit a run
			//output the old span if we had one
			if(nSpanLen > 0)
			{
				OutputSpan(false, nSpanLen, &pData[nCurrPel - nSpanLen * 3], pOutPos);
			}

			//output the run span
			OutputSpan(true, nRunLen, &pData[nCurrPel], pOutPos);
			
			//start the new span
			nSpanLen = 0;

			//update the offset (the -1 is to counteract the loop increment)
			nCurrPel += (nRunLen - 1) * 3;
		}
		else
		{
			//no run, so add this onto the current span
			nSpanLen++;
		}

		//check to see if we have to output a span
		if(nSpanLen > 127)
		{
			//print out the span
			OutputSpan(false, nSpanLen, &pData[nCurrPel - nSpanLen * 3], pOutPos);

			//reset the span
			nSpanLen = 0;
		}
	}

	//make sure we finished off the final span
	if(nSpanLen > 0)
	{
		OutputSpan(false, nSpanLen, &pData[nCurrPel - nSpanLen * 3], pOutPos);
	}

	//find the length of the data we encoded
	outLen = (uint32)(pOutPos - pOutBuffer);

	ASSERT(outLen <= LIGHTMAP_MAX_DATA_SIZE);

	return LTTRUE;
}

// Decompresses the data in pCompressed, storing it in pOut.
// pOut MUST be (LIGHTMAP_MAX_TOTAL_PIXELS * 3) in length.
// returns the success code.
bool DecompressLMData(uint8 *pCompressed, uint32 dataLen, uint8 *pOut)
{
	//sanity checks
	if((pCompressed == NULL) || (pOut == NULL))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	//the index into the input buffer
	uint32 nCurrPos = 0;

	//cursor in the output buffer
	uint8* pCurrOut = pOut;

	//run through the input buffer
	for(; nCurrPos < dataLen; )
	{
		//read in the tag
		uint8 nTag = pCompressed[nCurrPos];
		nCurrPos++;

		//see if it is a run or a span
		bool bIsRun = (nTag & 0x80) ? true : false;
		
		//blit the color span
		uint32 nRunLen = (uint32)(nTag & 0x7F) + 1;

		for(uint32 nCurrPel = 0; nCurrPel < nRunLen; nCurrPel++)
		{
			//set the color
			pCurrOut[0] = pCompressed[nCurrPos + 0];
			pCurrOut[1] = pCompressed[nCurrPos + 1];
			pCurrOut[2] = pCompressed[nCurrPos + 2];

			//update the output position
			pCurrOut += 3;

			//if it isn't a run, we need to move on to the next input color
			if(!bIsRun)
			{
				//update the input position
				nCurrPos += 3;
			}

		}

		//if this was a run, we need to move onto the next byte now
		if(bIsRun)
		{
			//update the input position
			nCurrPos += 3;
		}		
	}

	//these should match up exactly
	ASSERT(nCurrPos == dataLen);

	return LTTRUE;
}

bool GetLMDataTexel(uint8 *pCompressed, uint32 nDataLen, uint32 nWidth, uint32 nX, uint32 nY, LTRGB *pOut)
{
	// Sanity checks
	if((pCompressed == NULL) || (pOut == NULL))
	{
		ASSERT(FALSE);
		return false;
	}

	// Find out how far into the "output data" this texel is
	uint32 nDataOfs = nY * nWidth + nX;

	// The index into the input buffer
	uint32 nCurrPos = 0;

	// How far into the output buffer we would be...
	uint32 nOutputPos = 0;

	// Run through the input buffer
	for(; nCurrPos < nDataLen; )
	{
		// Read in the tag
		uint8 nTag = pCompressed[nCurrPos];
		nCurrPos++;

		// Is this a run?
		bool bIsRun = (nTag & 0x80) ? true : false;
		
		// Get the run length
		uint32 nRunLen = (uint32)(nTag & 0x7F) + 1;

		// Update the output data position
		nOutputPos += nRunLen;

		// If we're past the texel we're looking for, we're done
		if (nOutputPos > nDataOfs)
		{
			// Set the color
			pOut->r = pCompressed[nCurrPos + 0];
			pOut->g = pCompressed[nCurrPos + 1];
			pOut->b = pCompressed[nCurrPos + 2];
			pOut->a = 255;

			// And jump out
			break;
		}

		// Update the input position
		if (bIsRun)
			nCurrPos += 3;
		else
			nCurrPos += 3 * nRunLen;
	}

	// If we didn't find the data offset we were looking for, it's past the end of the data
	if (nOutputPos < nDataOfs)
	{
		ASSERT(FALSE);
		return false;
	}

	return true;
}
