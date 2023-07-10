//! Utilities for reading binary data.

// (C) Gennady Trafimenkov

use std::io;
use std::io::Read;

pub trait ByteOrderReader {
    fn read_i16(&mut self) -> io::Result<i16>;
    fn read_u8(&mut self) -> io::Result<u8>;
    fn read_u16(&mut self) -> io::Result<u16>;
    fn read_u32(&mut self) -> io::Result<u32>;
}

/// LittleEndianReader is a thin wrapper around io::Read that allows reading
/// binary data stored in little-endian format.
pub struct LittleEndianReader<'a> {
    pub reader: &'a mut dyn Read,
}

/// BigEndianReader is a thin wrapper around io::Read that allows reading
/// binary data stored in little-endian format.
pub struct BigEndianReader<'a> {
    pub reader: &'a mut dyn Read,
}

impl<'a> LittleEndianReader<'a> {
    pub fn new(reader: &'a mut dyn Read) -> Self {
        Self { reader }
    }
}

impl ByteOrderReader for LittleEndianReader<'_> {
    fn read_i16(&mut self) -> io::Result<i16> {
        let val = self.read_u16()?;
        Ok(val as i16)
    }

    fn read_u8(&mut self) -> io::Result<u8> {
        let mut buf = [0; 1];
        self.reader.read_exact(&mut buf)?;
        Ok(buf[0])
    }

    fn read_u16(&mut self) -> io::Result<u16> {
        let mut buf = [0; 2];
        self.reader.read_exact(&mut buf)?;
        Ok((buf[0] as u16) | (buf[1] as u16) << 8)
    }

    fn read_u32(&mut self) -> io::Result<u32> {
        let mut buf = [0; 4];
        self.reader.read_exact(&mut buf)?;
        Ok((buf[0] as u32) | (buf[1] as u32) << 8 | (buf[2] as u32) << 16 | (buf[3] as u32) << 24)
    }
}

impl<'a> BigEndianReader<'a> {
    #[allow(dead_code)]
    pub fn new(reader: &'a mut dyn Read) -> Self {
        Self { reader }
    }
}

impl ByteOrderReader for BigEndianReader<'_> {
    fn read_i16(&mut self) -> io::Result<i16> {
        let val = self.read_u16()?;
        Ok(val as i16)
    }

    fn read_u8(&mut self) -> io::Result<u8> {
        let mut buf = [0; 1];
        self.reader.read_exact(&mut buf)?;
        Ok(buf[0])
    }

    fn read_u16(&mut self) -> io::Result<u16> {
        let mut buf = [0; 2];
        self.reader.read_exact(&mut buf)?;
        Ok((buf[0] as u16) << 8 | (buf[1] as u16))
    }

    fn read_u32(&mut self) -> io::Result<u32> {
        let mut buf = [0; 4];
        self.reader.read_exact(&mut buf)?;
        Ok((buf[0] as u32) << 24 | (buf[1] as u32) << 16 | (buf[2] as u32) << 8 | (buf[3] as u32))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read() {
        let buffer = [
            0x01, 0x02, 0x03, 0x04, // testing u32
            0x05, // testing u8
            0x06, 0x07, // testing u16
            0xf0, 0xd8, // testing i16
        ];

        {
            let mut cursor = io::Cursor::new(buffer);
            let mut reader = LittleEndianReader::new(&mut cursor);
            assert_eq!(0x04030201, reader.read_u32().unwrap());
            assert_eq!(0x05, reader.read_u8().unwrap());
            assert_eq!(0x0706, reader.read_u16().unwrap());
            assert_eq!(-10000, reader.read_i16().unwrap());
        }

        {
            let mut cursor = io::Cursor::new(buffer);
            let mut reader = BigEndianReader::new(&mut cursor);
            assert_eq!(0x01020304, reader.read_u32().unwrap());
            assert_eq!(0x05, reader.read_u8().unwrap());
            assert_eq!(0x0607, reader.read_u16().unwrap());
            assert_eq!(-3880, reader.read_i16().unwrap());
        }
    }
}
