import sys
from lib import EDM

# 1. read a test binary file and get the 1st event
file = open("Klaus6_bitflow_test.txt", "rb")
binary_file = file.read()
file.close()

bytes_first_event = binary_file[0:60]

# 2. feed the first event to EDM class
first_event = EDM.EDM(bytes_first_event)
first_event.printHeader()
first_event.print() # the output is the same as that from the c++ daq-i2c class 

# 3. try to read 10 events
for i in range(10):
	bytes_i_event = binary_file[60*i:60*(i+1)]
	i_event = EDM.EDM(bytes_i_event)
	i_event.print()

# To integrate with klaus6.py:
# 1. Replace the source of the binary file from test file to the output of klaus6.py: 
#	 binary_file = klaus6.readEvents()
# 2. Repeat step 2 and 3
