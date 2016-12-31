import random
import sys

sys.stdout.write("0x")
for i in range(0,8):
    sys.stdout.write("%02x" % (random.randint(0,255)))
sys.stdout.write("ULL\n")



