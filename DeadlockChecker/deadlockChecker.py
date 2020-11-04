import random
import copy
import math
import redis
import json
import time

r = redis.Redis(host="localhost", port=6379, db=0)

robot_positions = []
robot_positions.append(-1)
robot_positions.append(-1)
robot_positions.append(-1)

robot_count = []
robot_count.append(0)
robot_count.append(0)
robot_count.append(0)

r.set("randomNumber", 1)

randomNumber = 1

while (True):
	for i in range(3):
		position = r.hget("robot_position", i + 1)
		if position == robot_positions[i]:
			robot_count[i] += 1
		else:
			robot_count[i] = 0
		robot_positions[i] = position
	cnt = 0
	for i in robot_count:
		if i > 20:
			cnt += 1
	if (cnt >= 2):
		robot_count = []
		robot_count.append(0)
		robot_count.append(0)
		robot_count.append(0)
		r.set("dead_lock", 1)
		r.set("randomNumber", randomNumber)
		randomNumber += 1
		print("dead_lock")
	print robot_count
	time.sleep(1)





