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
 
#include <libs/osc/reader/types/OscAddress.h>
#include <libs/osc/exceptions/BooleanConversionException.h>
#include <libs/osc/exceptions/BytesConversionException.h>
#include <libs/osc/exceptions/CharConversionException.h>
#include <libs/osc/exceptions/DoubleConversionException.h>
#include <libs/osc/exceptions/FloatConversionException.h>
#include <libs/osc/exceptions/IntegerConversionException.h>
#include <libs/osc/exceptions/LongConversionException.h>
#include <libs/osc/exceptions/MidiConversionException.h>
#include <libs/osc/exceptions/RgbaConversionException.h>
#include <libs/osc/exceptions/SymbolConversionException.h>
#include <libs/osc/exceptions/TimetagConversionException.h>

/**
 * Build a new OscAddress from the given location in packet.
 *
 * @param pos
 *            an address pattern string position.
 */
OscAddress::OscAddress(ByteBuffer* packet, qint32 pos)
: OscValue('\0', packet, pos)
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
 * Returns the address pattern string.
 *
 * @return a string
 */
QString OscAddress::get()
{
	try
	{
		return OscValue::getString(mPacket, mPos);
	}
	catch (const QException& e)
	{
		throw e;
	}
}

bool OscAddress::toBoolean()
{
	throw BooleanConversionException();
}

QByteArray OscAddress::toBytes()
{
	throw BytesConversionException();
}

char OscAddress::toChar()
{
	throw CharConversionException();
}

double OscAddress::toDouble()
{
	throw DoubleConversionException();
}

float OscAddress::toFloat()
{
	throw FloatConversionException();
}

qint32 OscAddress::toInteger()
{
	throw IntegerConversionException();
}

qint64 OscAddress::toLong()
{
	throw LongConversionException();
}


QString OscAddress::toString()
{
	return get();
}
