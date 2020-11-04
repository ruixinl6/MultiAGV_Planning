import random
import copy
import math
import redis
import json
import time

r = redis.Redis(host="localhost", port=6379, db=0)
robot_travelper = [0.0, 0.0, 0.0]

physical_node_list = r.lrange("physical_node_list", 0, -1)
physical_link_list = r.lrange("physical_link_list", 0, -1)
logical_node_list = r.lrange("logical_node_list", 0, -1)

for i in range(len(physical_node_list)):
	physical_node_list[i] = json.loads(physical_node_list[i])

for i in range(len(physical_link_list)):
	physical_link_list[i] = json.loads(physical_link_list[i])

for i in range(len(logical_node_list)):
	logical_node_list[i] = json.loads(logical_node_list[i])



while (True):
	for i in range(3):
		with_box = int(r.hget("robot_with_box", i + 1))
		current_position = int(r.hget("robot_position", i + 1))
		vps = r.hget("robot_vpoint", i + 1)
		vps = vps.split("|")
		for j in range(len(vps)):
			vps[j] = int(vps[j])
		cfun = r.hget("robot_cFuns", i + 1)
		cfun = cfun.split("|")

		flag = 1
		for j in range(len(cfun)):
			if (cfun[j] == ''):
				flag = 2
				break
			cfun[j] = int(cfun[j])

		if (flag == 2):
			continue
		#print "robot cfun ", i + 1, " : ", cfun

		index = -1
		for j in range(len(cfun)):
			if cfun[j] == current_position:
				index = j
				break

		#print "robot Index ", i + 1, " : ", index

		if (index < len(cfun) - 1):
			next_position = int(cfun[index + 1])
			if (abs(physical_node_list[next_position]["x"] - physical_node_list[current_position]["x"]) > 1\
				or abs(physical_node_list[next_position]["y"] - physical_node_list[current_position]["y"]) > 1\
				):
				current_x = physical_node_list[current_position]["x"]
				current_y = physical_node_list[current_position]["y"]
				next_x = physical_node_list[next_position]["x"]
				next_y = physical_node_list[next_position]["y"]
				real_x = current_x + (next_x - current_x) * robot_travelper[i]
				real_y = current_y + (next_y - current_y) * robot_travelper[i]
				r.hset("robot_x", i + 1, real_x)
				r.hset("robot_y", i + 1, real_y)

				robot_travelper[i] += 0.1
				print "travelper of ", i + 1, " : ", robot_travelper[i]

				if (robot_travelper[i] >= 1):
					robot_travelper[i] = 0
					current_position = next_position
					print "robot ", i + 1, " : ", current_position
					r.hset("robot_position", i + 1, current_position)

			else:
				current_position = next_position
				print "robot ", i + 1, " : ", current_position
				r.hset("robot_position", i + 1, current_position)
				current_x = physical_node_list[current_position]["x"]
				current_y = physical_node_list[current_position]["y"]
				real_x = current_x
				real_y = current_y
				r.hset("robot_x", i + 1, real_x)
				r.hset("robot_y", i + 1, real_y)

			if (current_position == vps[-1]):
				task = json.loads(r.hget("robot_task", i + 1))
				task["from"] = task["from"]
				task["to"] = task["to"]
				task["isFinished"] = 1
				r.hset("robot_task", i + 1, json.dumps(task, ensure_ascii = False))
				print "robot ", i + 1, " finish one job "
		
		if (current_position == vps[-1]):
			task = json.loads(r.hget("robot_task", i + 1))
			task["from"] = task["from"]
			task["to"] = task["to"]
			task["isFinished"] = 1
			if (with_box == 1):
				r.hset("robot_with_box", i + 1, 0)
			if (with_box == 0):
				r.hset("robot_with_box", i + 1, 1)
			r.hset("robot_task", i + 1, json.dumps(task, ensure_ascii = False))
			print "robot ", i + 1, " finish one job "

		
	time.sleep(1)