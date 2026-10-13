/*
 * Copyright (c) 2014 MUGEN SAS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include <libs/osc/reader/types/OscBlob.h>
#include <libs/tools/ByteBuffer.h>
#include <libs/osc/exceptions/CharConversionException.h>
#include <libs/osc/exceptions/DoubleConversionException.h>
#include <libs/osc/exceptions/IntegerConversionException.h>
#include <libs/osc/exceptions/FloatConversionException.h>
#include <libs/osc/exceptions/LongConversionException.h>
#include <libs/osc/exceptions/StringConversionException.h>

OscBlob::OscBlob(ByteBuffer* packet, qint32 pos)
: OscValue('b', packet, pos)
{
	try
	{
		get();
	}
	catch (const QException& e)
	{
		throw e;
	}
}

/**
 * Returns the blob data.
 *
 * @return a blob data.
 */
QByteArray& OscBlob::get()
{
	try
	{
		mBlobSize = mPacket->getInt(mPos);
        // Validate the length and padding before allocating from network input.
        const qint32 remaining = mPacket->getCapacity() - mPos - 4;
        if (mBlobSize < 0 || mBlobSize > remaining)
            throw BufferUnderflowException();
        const qint32 padding = (4 - (mBlobSize % 4)) % 4;
        if (padding > remaining - mBlobSize)
            throw BufferUnderflowException();

        mData = QByteArray(mBlobSize, 0);
        mPacket->setPosition(mPos + 4);
        mPacket->get(&mData, 0, mBlobSize);
        mPacket->setPosition(mPos + 4 + mBlobSize + padding);

		return mData;
	}
	catch (const QException& e)
	{
		throw e;
	}
}

bool OscBlob::toBoolean()
{
	return (get().length() != 0);
}

QByteArray OscBlob::toBytes()
{
	return get();
}

char OscBlob::toChar()
{
	throw CharConversionException();
}

double OscBlob::toDouble()
{
	throw DoubleConversionException();
}

float OscBlob::toFloat()
{
	throw FloatConversionException();
}

qint32 OscBlob::toInteger()
{
	throw IntegerConversionException();
}

qint64 OscBlob::toLong()
{
	throw LongConversionException();
}

QString OscBlob::toString()
{
	throw StringConversionException();
}
