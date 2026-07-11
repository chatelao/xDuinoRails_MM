//
// Copyright (c) 2024 Mateusz Stadnik
// Copyright (c) 2010-2024 Antmicro
// Copyright (c) 2011-2015 Realtime Embedded
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

using System;
using System.Linq;
using Antmicro.Renode.Debugging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Peripherals.Memory;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Utilities.Packets;

namespace Antmicro.Renode.Peripherals.DMA
{

  public struct RPXXXXDmaRequest
  {
    public RPXXXXDmaRequest(Request request, int ringSize, bool ringWrite, int offset)
      : this()
    {
      this.request = request;
      this.ringSize = ringSize;
      this.ringWrite = ringWrite;
      this.offset = offset;
    }
    public Request request;
    public int ringSize;
    public bool ringWrite;
    public int offset;
  }

  public struct ChecksumRequest
  {
    public enum Type
    {
      Crc32,
      Crc32Reversed,
      Crc16CCITT,
      Crc16CCITTReversed,
      XORReduction,
      Sum
    };
    public Type type { get; set; }
    public uint init { get; set; }
  }

  public struct ResponseWithCrc
  {
    public Response response { get; set; }
    public uint crc { get; set; }
  }

  public sealed class RPDmaEngine
  {
    public RPDmaEngine(IBusController systemBus)
    {
      sysbus = systemBus;
    }

    public ResponseWithCrc IssueCopy(RPXXXXDmaRequest request, CPU.ICPU context = null, ChecksumRequest? checksum = null)
    {
      var response = new Response
      {
        ReadAddress = request.request.Source.Address,
        WriteAddress = request.request.Destination.Address,
      };

      var responseWithCrc = new ResponseWithCrc();
      var readLengthInBytes = (int)request.request.ReadTransferType;
      var writeLengthInBytes = (int)request.request.WriteTransferType;

      ulong readOffset = 0;
      ulong writeOffset = 0;
      if (request.request.IncrementReadAddress)
      {
        readOffset = (ulong)request.offset;
        if (!request.ringWrite && request.ringSize != 0)
        {
          readOffset = readOffset % (ulong)request.ringSize;
        }
      }

      if (request.request.IncrementWriteAddress)
      {
        writeOffset = (ulong)request.offset;
        if (request.ringWrite && request.ringSize != 0)
        {
          writeOffset = writeOffset % (ulong)request.ringSize;
        }
      }
      writeOffset *= (ulong)writeLengthInBytes;
      readOffset *= (ulong)readLengthInBytes;

      // some sanity checks
      if ((request.request.Size % readLengthInBytes) != 0 || (request.request.Size % writeLengthInBytes) != 0)
      {
        throw new ArgumentException("Request size is not aligned properly to given read or write transfer type (or both).");
      }

      var buffer = new byte[request.request.Size];
      var sourceAddress = request.request.Source.Address ?? 0;
      var whatIsAtSource = sysbus.WhatIsAt(sourceAddress, context);
      var isSourceContinuousMemory = (whatIsAtSource == null || whatIsAtSource.Peripheral is MappedMemory) // Not a peripheral
                                                  && (ulong)readLengthInBytes == request.request.SourceIncrementStep; // Consistent memory region
      if (!request.request.Source.Address.HasValue)
      {
        // request array based copy
        Array.Copy(request.request.Source.Array, request.request.Source.StartIndex.Value, buffer, 0, request.request.Size);
      }
      else if (isSourceContinuousMemory)
      {
        if (request.request.IncrementReadAddress)
        {
          // Transfer Units |  1  |  2  |  3  |  4  |
          // Source         |  A  |  B  |  C  |  D  |
          // Copied         |  A  |  B  |  C  |  D  |
          response.ReadAddress = ReadFromMemory(sourceAddress + readOffset, buffer, request.request.Size, context, request.ringWrite == false ? request.ringSize : 0);
        }
        else
        {
          // When reading from the memory with IncrementReadAddress unset, effectively, only the last unit will be used
          // Transfer Units |  1  |  2  |  3  |  4  |
          // Source         |  A  |  B  |  C  |  D  |
          // Copied         |  D  |     |     |     |
          sysbus.ReadBytes(sourceAddress + readOffset, readLengthInBytes, buffer, 0, context: context);
        }
      }
      else if (whatIsAtSource != null)
      {
        // Read from peripherals
        var transferred = 0;
        var currentReadAddr = sourceAddress + readOffset;
        int ringSize = request.ringWrite == false ? request.ringSize : 0;
        ulong mask = (ulong)ringSize - 1;
        ulong baseAddr = currentReadAddr & ~mask;

        while (transferred < request.request.Size)
        {
          switch (request.request.ReadTransferType)
          {
            case TransferType.Byte:
              buffer[transferred] = sysbus.ReadByte(currentReadAddr, context);
              break;
            case TransferType.Word:
              BitConverter.GetBytes(sysbus.ReadWord(currentReadAddr, context)).CopyTo(buffer, transferred);
              break;
            case TransferType.DoubleWord:
              BitConverter.GetBytes(sysbus.ReadDoubleWord(currentReadAddr, context)).CopyTo(buffer, transferred);
              break;
            case TransferType.QuadWord:
              BitConverter.GetBytes(sysbus.ReadQuadWord(currentReadAddr, context)).CopyTo(buffer, transferred);
              break;
            default:
              throw new ArgumentOutOfRangeException($"Requested read transfer size: {request.request.ReadTransferType} is not supported by DmaEngine");
          }
          transferred += readLengthInBytes;
          if (request.request.IncrementReadAddress)
          {
            if (ringSize != 0)
            {
                currentReadAddr = baseAddr | ((currentReadAddr + request.request.SourceIncrementStep) & mask);
            }
            else
            {
                currentReadAddr += request.request.SourceIncrementStep;
            }
            response.ReadAddress = currentReadAddr;
          }
        }
      }

      if (checksum != null)
      {
        switch (checksum.Value.type)
        {
          case ChecksumRequest.Type.Crc32Reversed:
            {
              var crcEngine = new CRCEngine(CRCPolynomial.CRC32, true, true, checksum.Value.init);
              responseWithCrc.crc = crcEngine.Calculate(buffer);
              break;
            }
          case ChecksumRequest.Type.Crc32:
            {
              var crcEngine = new CRCEngine(CRCPolynomial.CRC32, false, false, checksum.Value.init);
              responseWithCrc.crc = crcEngine.Calculate(buffer);
              break;
            }
          case ChecksumRequest.Type.Crc16CCITT:
            {
              var crcEngine = new CRCEngine(CRCPolynomial.CRC16_CCITT, false, false, checksum.Value.init);
              responseWithCrc.crc = crcEngine.Calculate(buffer);
              break;
            }
          case ChecksumRequest.Type.Crc16CCITTReversed:
            {
              var crcEngine = new CRCEngine(CRCPolynomial.CRC16_CCITT, true, false, checksum.Value.init);
              responseWithCrc.crc = crcEngine.Calculate(buffer);
              break;
            }
          case ChecksumRequest.Type.XORReduction:
            {
              UInt32 calculated = checksum.Value.init;
              foreach (var b in buffer)
              {
                calculated ^= b;
              }
              uint bitCount = 0;
              while (calculated > 0)
              {
                bitCount += calculated & 1;
                calculated >>= 1;
              }
              responseWithCrc.crc = Convert.ToUInt32(bitCount % 2 == 1);
              break;
            }
          case ChecksumRequest.Type.Sum:
            {
              uint sum = 0;
              foreach (var b in buffer)
              {
                sum += b;
              }
              responseWithCrc.crc = sum;
              break;
            }
        }
      }

      var destinationAddress = request.request.Destination.Address ?? 0;
      var whatIsAtDestination = sysbus.WhatIsAt(destinationAddress);
      var isDestinationContinuousMemory = (whatIsAtDestination == null || whatIsAtDestination.Peripheral is MappedMemory) // Not a peripheral
                                                  && (ulong)readLengthInBytes == request.request.DestinationIncrementStep;  // Consistent memory region
      if (!request.request.Destination.Address.HasValue)
      {
        // request array based copy
        Array.Copy(buffer, 0, request.request.Destination.Array, request.request.Destination.StartIndex.Value, request.request.Size);
      }
      else if (isDestinationContinuousMemory)
      {
        if (request.request.IncrementWriteAddress)
        {
          int ringSize = request.ringWrite == true ? request.ringSize : 0;
          if (request.request.IncrementReadAddress || !isSourceContinuousMemory)
          {
            // Transfer Units |  1  |  2  |  3  |  4  |
            // Source         |  A  |  B  |  C  |  D  |
            // Destination    |  A  |  B  |  C  |  D  |
            response.WriteAddress = WriteToMemory(destinationAddress + writeOffset, buffer, context, ringSize);
          }
          else
          {
            // When writing memory with IncrementReadAddress unset all destination units are written with the first source unit
            // Transfer Units |  1  |  2  |  3  |  4  |
            // Source         |  A  |  B  |  C  |  D  |
            // Destination    |  A  |  A  |  A  |  A  |
            var chunkStartOffset = 0UL;
            var chunk = new byte[writeLengthInBytes];
            Array.Copy(buffer, 0, chunk, 0, writeLengthInBytes);

            ulong currentWriteAddr = destinationAddress + writeOffset;
            ulong mask = (ulong)ringSize - 1;
            ulong baseAddr = currentWriteAddr & ~mask;

            while (chunkStartOffset < (ulong)request.request.Size)
            {
              int chunkSize = writeLengthInBytes;
              if (ringSize != 0)
              {
                  ulong offsetInRing = currentWriteAddr & mask;
                  int bytesToBoundary = ringSize - (int)offsetInRing;
                  chunkSize = Math.Min(writeLengthInBytes, bytesToBoundary);
              }

              sysbus.WriteBytes(chunk, currentWriteAddr, (long)chunkSize, false, context: context);

              if (ringSize != 0)
              {
                  currentWriteAddr = baseAddr | ((currentWriteAddr + (ulong)chunkSize) & mask);
              }
              else
              {
                  currentWriteAddr += (ulong)chunkSize;
              }
              chunkStartOffset += (ulong)chunkSize;
            }
            response.WriteAddress = currentWriteAddr;
          }
        }
        else
        {
          // When writing to memory with IncrementWriteAddress unset, effectively, only the last unit is written with the last unit of source
          // Transfer Units |  1  |  2  |  3  |  4  |
          // Source         |  A  |  B  |  C  |  D  |
          // Destination    |  D  |     |     |     |
          var skipCount = (request.request.Size == writeLengthInBytes) ? 0 : request.request.Size - writeLengthInBytes;
          DebugHelper.Assert((skipCount + request.request.Size) <= buffer.Length);
          var lastUnit = new byte[writeLengthInBytes];
          Array.Copy(buffer, skipCount, lastUnit, 0, writeLengthInBytes);
          sysbus.WriteBytes(lastUnit, destinationAddress + writeOffset, context: context);
        }
      }
      else if (whatIsAtDestination != null)
      {
        // Write to peripheral
        var transferred = 0;
        var currentWriteAddr = destinationAddress + writeOffset;
        int ringSize = request.ringWrite == true ? request.ringSize : 0;
        ulong mask = (ulong)ringSize - 1;
        ulong baseAddr = currentWriteAddr & ~mask;

        while (transferred < request.request.Size)
        {
          switch (request.request.WriteTransferType)
          {
            case TransferType.Byte:
              sysbus.WriteByte(currentWriteAddr, buffer[transferred], context);
              break;
            case TransferType.Word:
              sysbus.WriteWord(currentWriteAddr, BitConverter.ToUInt16(buffer, transferred), context);
              break;
            case TransferType.DoubleWord:
              sysbus.WriteDoubleWord(currentWriteAddr, BitConverter.ToUInt32(buffer, transferred), context);
              break;
            case TransferType.QuadWord:
              sysbus.WriteQuadWord(currentWriteAddr, BitConverter.ToUInt64(buffer, transferred), context);
              break;
            default:
              throw new ArgumentOutOfRangeException($"Requested write transfer size: {request.request.WriteTransferType} is not supported by DmaEngine");
          }
          transferred += writeLengthInBytes;
          if (request.request.IncrementWriteAddress)
          {
            if (ringSize != 0)
            {
                currentWriteAddr = baseAddr | ((currentWriteAddr + request.request.DestinationIncrementStep) & mask);
            }
            else
            {
                currentWriteAddr += request.request.DestinationIncrementStep;
            }
            response.WriteAddress = currentWriteAddr;
          }
        }
      }

      responseWithCrc.response = response;
      return responseWithCrc;
    }

    private ulong ReadFromMemory(ulong sourceAddress, byte[] buffer, int size, CPU.ICPU context, int ringSize)
    {
      if (ringSize == 0)
      {
        sysbus.ReadBytes(sourceAddress, size, buffer, 0, context: context);
        return sourceAddress + (ulong)size;
      }

      ulong mask = (ulong)ringSize - 1;
      ulong baseAddr = sourceAddress & ~mask;
      ulong currentAddr = sourceAddress;

      int transferred = 0;
      while (transferred < size)
      {
        ulong offsetInRing = currentAddr & mask;
        int bytesToBoundary = ringSize - (int)offsetInRing;
        int chunkSize = Math.Min(size - transferred, bytesToBoundary);

        sysbus.ReadBytes(currentAddr, chunkSize, buffer, transferred, context: context);

        transferred += chunkSize;
        currentAddr = baseAddr | ((currentAddr + (ulong)chunkSize) & mask);
      }
      return currentAddr;
    }

    private ulong WriteToMemory(ulong destinationAddress, byte[] buffer, CPU.ICPU context, int ringSize)
    {
      int size = buffer.Length;
      if (ringSize == 0)
      {
        sysbus.WriteBytes(buffer, destinationAddress, context: context);
        return destinationAddress + (ulong)size;
      }

      ulong mask = (ulong)ringSize - 1;
      ulong baseAddr = destinationAddress & ~mask;
      ulong currentAddr = destinationAddress;

      int transferred = 0;
      while (transferred < size)
      {
        ulong offsetInRing = currentAddr & mask;
        int bytesToBoundary = ringSize - (int)offsetInRing;
        int chunkSize = Math.Min(size - transferred, bytesToBoundary);

        var chunk = new byte[chunkSize];
        Array.Copy(buffer, transferred, chunk, 0, chunkSize);
        sysbus.WriteBytes(chunk, currentAddr, context: context);

        transferred += chunkSize;
        currentAddr = baseAddr | ((currentAddr + (ulong)chunkSize) & mask);
      }
      return currentAddr;
    }


    private readonly IBusController sysbus;
  }
}
