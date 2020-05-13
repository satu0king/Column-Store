
import random



for i in range(100000):
    print("INSERT INTO TABLE1 VALUES (" + str(i) + "," + str(random.sample(range(0, 1000000), 100))[1:-1] + ');')