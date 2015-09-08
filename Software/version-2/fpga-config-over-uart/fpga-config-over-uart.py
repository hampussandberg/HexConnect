# *******************************************************************************
# * @file    fpga-config-over-uart.py
# * @author  Hampus Sandberg
# * @version 0.1
# * @date    2015-09-08
# * @brief
# *******************************************************************************
#  Copyright (c) 2015 Hampus Sandberg.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *******************************************************************************

import serial
import sys
import getopt
import binascii

class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'

def main(argv):
  serPort = ''
  startAddr = ''
  try:
    opts, args = getopt.getopt(argv,"hp:a:",["serialPort=","startAddress="])
  except getopt.GetoptError:
    print "fpga-config-over-uart.py -p <serialPort> -a <startAddress, example 12AB72FE>"
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print "fpga-config-over-uart.py -p <serialPort> -a <startAddress, example 12AB72FE>"
      sys.exit()
    elif opt in ("-p", "--serialPort"):
      serPort = arg
    elif opt in ("-a", "--startAddress"):
      startAddr = arg
  serialSend(serPort, startAddr)


def serialSend(serialPort, startAddress):
  # Convert to bytearray
  startAddressAsByteArray = bytearray.fromhex(startAddress)

  # Open the serial port
  ser = serial.Serial(serialPort, 115200, timeout=10)
  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC
    raw_input(bcolors.OKBLUE + "Press Enter to start sending data..." + bcolors.ENDC)

  # Set the flash write address to 0x00000000
  print "Sending write address command"
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x10) ^ int(0x04))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[0]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[1]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[2]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[3]))
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x04])
  writeAddressCommand.extend(startAddressAsByteArray)
  writeAddressCommand.extend(checksum)
  ser.write(writeAddressCommand)
  # Wait for ack
  response = ser.read(1)
  if (response and ord(response) == int(0xDD)):
    print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
  else:
    print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
    sys.exit()


  # Read the bitfile
  with open("test.sof", "rb") as f:
    byte = 1
    checksum = 0
    count = 0
    data = bytearray()
    iterationCount = 0
    while byte:
      # Do stuff with byte.
      byte = f.read(1)
      if (byte):
        data.extend(byte)
        count += 1
        if (count == 1):
          checksum = byte
        else:
          checksum = chr(ord(checksum) ^ ord(byte))

        if (count == 128):
          iterationCount += 1
          print(iterationCount),
          print(":"),
          print(count),
          print("bytes read..."),

          # Add the header, command and data count to the checksum
          checksum = chr(ord(checksum) ^ int(0xAA))
          checksum = chr(ord(checksum) ^ int(0xBB))
          checksum = chr(ord(checksum) ^ int(0xCC))
          checksum = chr(ord(checksum) ^ int(0x30))
          checksum = chr(ord(checksum) ^ count)
          print(" checksum:"),
          print "0x" + binascii.hexlify(checksum),
          print("..."),

          # Construct the message
          msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, count])
          msg.extend(data)
          msg.extend(checksum)
          # DEBUG: Print the message
          #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC
          #raw_input(bcolors.OKBLUE + "Stop a bit" + bcolors.ENDC)

          # Send the message
          print("sending data..."),
          ser.write(msg)

          # Wait for ack
          response = ser.read(1)
          if (response and ord(response) == int(0xDD)):
            print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
          else:
            print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
            sys.exit()
            #raw_input(bcolors.OKBLUE + "Press Enter to continue..." + bcolors.ENDC)
          # Clear the data that has been sent
          del data[0:count]
          count = 0

    # Send the last bytes if there are any
    if (count != 0):
      print(count),
      print(" bytes left to send..."),
      # Add the header, command and data count to the checksum
      checksum = chr(ord(checksum) ^ int(0xAA))
      checksum = chr(ord(checksum) ^ int(0xBB))
      checksum = chr(ord(checksum) ^ int(0xCC))
      checksum = chr(ord(checksum) ^ int(0x30))
      checksum = chr(ord(checksum) ^ count)
      print(" checksum:"),
      print "0x" + binascii.hexlify(checksum),
      print("..."),

      # Construct the message
      msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, count])
      msg.extend(data)
      msg.extend(checksum)
      # DEBUG: Print the message
      #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC

      # Send the message
      print("sending data..."),
      ser.write(msg)

      # Wait for ack
      response = ser.read(1)
      if (response and ord(response) == int(0xDD)):
        print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
      else:
        print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
        sys.exit()
        #raw_input(bcolors.OKBLUE + "Press Enter to continue..." + bcolors.ENDC)


    print bcolors.OKGREEN + "Done sending data" + bcolors.ENDC



if __name__ == "__main__":
  main(sys.argv[1:])
