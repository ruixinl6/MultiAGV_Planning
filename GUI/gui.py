# events-example0.py
# Barebones timer, mouse, and keyboard events

from Tkinter import *
import random
import copy
import math
import redis
import json

def init(data):
	data.r = redis.Redis(host="localhost", port=6379, db=0)
	data.physical_node_list = data.r.lrange("physical_node_list", 0, -1)
	data.physical_link_list = data.r.lrange("physical_link_list", 0, -1)
	data.logical_node_list = data.r.lrange("logical_node_list", 0, -1)
	
	for i in range(len(data.physical_node_list)):
		data.physical_node_list[i] = json.loads(data.physical_node_list[i])

	for i in range(len(data.physical_link_list)):
		data.physical_link_list[i] = json.loads(data.physical_link_list[i])

	for i in range(len(data.logical_node_list)):
		data.logical_node_list[i] = json.loads(data.logical_node_list[i])



	print data.physical_node_list
	print data.physical_link_list
	print data.logical_node_list

	data.node_r = 15
	data.left = 50
	data.up = 50
	data.alpha = 35

	data.lock_type = "node"
	data.lock_nodes = []

	data.color = ["red", "green", "brown"]

	data.mapper_occupied_area = []
	data.robot_positions = []

	data.robot_x = []
	data.robot_y = []



	pass

def mousePressed(event, data):
	currX = event.x
	currY = event.y
	if (data.lock_type == "node"):
		distance = 10000
		lock_nodes = []
		for i in data.physical_node_list:
			if (abs(i["x"] * data.alpha + data.left - currX)\
				+ abs(i["y"] * data.alpha + data.up - currY)\
				< distance):
				lock_nodes = []
				for j in i["lock_nodes"]:
					lock_nodes.append(data.physical_node_list[j])
				distance = abs(i["x"] * data.alpha + data.left - currX)\
				+ abs(i["y"] * data.alpha + data.up - currY)
		if (distance < 30):
			data.lock_nodes = lock_nodes
	elif (data.lock_type == "link"):
		distance = 10000
		lock_nodes = []
		for i in data.physical_link_list:
			x1 = data.physical_node_list[i["from_physical_node"]]["x"]
			y1 = data.physical_node_list[i["from_physical_node"]]["y"]
			x2 = data.physical_node_list[i["to_physical_node"]]["x"]
			y2 = data.physical_node_list[i["to_physical_node"]]["y"]
			if ((x1 * data.alpha + data.left - currX) * (x2 * data.alpha + data.left - currX)
			 + (y1 * data.alpha + data.up - currY) * (y2 * data.alpha + data.up - currY) > 0):
				continue
			if (abs(x1 * data.alpha + data.left - currX)\
				+ abs(y1 * data.alpha + data.up - currY)\
				+ abs(x2 * data.alpha + data.left - currX)\
				+ abs(y2 * data.alpha + data.up - currY)\
				< distance):
				lock_nodes = []

				for j in data.physical_node_list[i["from_physical_node"]]["lock_nodes"]:
					if (data.physical_node_list[j] not in lock_nodes):
						lock_nodes.append(data.physical_node_list[j])
				for j in data.physical_node_list[i["to_physical_node"]]["lock_nodes"]:
					if (data.physical_node_list[j] not in lock_nodes):
						lock_nodes.append(data.physical_node_list[j])

				distance = abs(x1 * data.alpha + data.left - currX)\
				+ abs(y1 * data.alpha + data.up - currY)\
				+ abs(x2 * data.alpha + data.left - currX)\
				+ abs(y2 * data.alpha + data.up - currY)
		print distance
		if (distance < 300):
			data.lock_nodes = lock_nodes
	pass

def keyPressed(event, data):
	if (event.keysym == "n"):
		data.lock_type = "node"
	if (event.keysym == "l"):
		data.lock_type = "link"
	pass

def timerFired(data):
	temp = data.r.get("mapper_occupied_area")
	temp = temp.split("|")
	for i in range(len(temp)):
		temp[i] = temp[i].split(",")
	for i in range(len(temp)):
		temp[i] = int(temp[i][1])
	data.mapper_occupied_area = copy.deepcopy(temp)

	temp = []
	temp.append(int(data.r.hget("robot_position", 1)))
	temp.append(int(data.r.hget("robot_position", 2)))
	temp.append(int(data.r.hget("robot_position", 3)))
	data.robot_positions = copy.deepcopy(temp)

	temp_x = []
	temp_x.append(float(data.r.hget("robot_x", 1)))
	temp_x.append(float(data.r.hget("robot_x", 2)))
	temp_x.append(float(data.r.hget("robot_x", 3)))
	data.robot_x = copy.deepcopy(temp_x)

	temp_y = []
	temp_y.append(float(data.r.hget("robot_y", 1)))
	temp_y.append(float(data.r.hget("robot_y", 2)))
	temp_y.append(float(data.r.hget("robot_y", 3)))
	data.robot_y = copy.deepcopy(temp_y)

	pass



def drawNodes(canvas, data):
	for i in data.physical_node_list:
		if (i["type"] == "3" or i["type"] == "4"):
			canvas.create_oval(i["x"] * data.alpha + data.left - data.node_r,\
						i["y"] * data.alpha + data.up - data.node_r,\
						i["x"] * data.alpha + data.left + data.node_r,\
						i["y"] * data.alpha + data.up + data.node_r,\
						fill = "black", width = 0)
		else:	
			canvas.create_oval(i["x"] * data.alpha + data.left - data.node_r,\
						i["y"] * data.alpha + data.up - data.node_r,\
						i["x"] * data.alpha + data.left + data.node_r,\
						i["y"] * data.alpha + data.up + data.node_r,\
						fill = "black", width = 0)
	for i in data.physical_node_list:
		curr_node = i["physical_node_id"]
		occupied_robot_id = data.mapper_occupied_area[i["physical_node_id"]];
		if (occupied_robot_id != 0):
			canvas.create_oval(i["x"] * data.alpha + data.left - data.node_r,\
						i["y"] * data.alpha + data.up - data.node_r,\
						i["x"] * data.alpha + data.left + data.node_r,\
						i["y"] * data.alpha + data.up + data.node_r,\
						fill = data.color[occupied_robot_id - 1], width = 0)
	for i in range(len(data.robot_x)):
		#curr_node = data.robot_positions[i]
		#temp_physical_node = data.physical_node_list[curr_node]
		temp_beta = 1.2
		canvas.create_rectangle(data.robot_x[i] * data.alpha + data.left - data.node_r * temp_beta,\
						data.robot_y[i] * data.alpha + data.up - data.node_r * temp_beta,\
						data.robot_x[i] * data.alpha + data.left + data.node_r * temp_beta,\
						data.robot_y[i] * data.alpha + data.up + data.node_r * temp_beta,\
						fill = data.color[i], width = 0)


def drawLinks(canvas, data):
	for i in data.physical_link_list:
		from_x = data.physical_node_list[i["from_physical_node"]]["x"]
		from_y = data.physical_node_list[i["from_physical_node"]]["y"]
		to_x = data.physical_node_list[i["to_physical_node"]]["x"]
		to_y = data.physical_node_list[i["to_physical_node"]]["y"]
		canvas.create_line(from_x * data.alpha + data.left,\
						from_y * data.alpha + data.up,\
						to_x * data.alpha + data.left,\
						to_y * data.alpha + data.up,\
						fill = "blue", width = 3)

def drawText(canvas, data):
	#text = "Select Lock Nodes from - " + data.lock_type +", TYPE 'l' or 'n' to select mode."
	#canvas.create_text(data.width / 2, data.height - 50, text = text, font = "time 10 bold", fill = "black")
	for i in data.physical_node_list:
		canvas.create_text(i["x"] * data.alpha + data.left,\
						i["y"] * data.alpha + data.up,\
						font = "time 5 bold",\
						text = str(i["physical_node_id"]),\
						fill = "yellow")
	# for i in data.physical_node_list:
	# 	curr_node = i["physical_node_id"]
	# 	occupied_robot_id = data.mapper_occupied_area[i["physical_node_id"]];
	# 	if (occupied_robot_id != 0):
	# 		canvas.create_text(i["x"] * data.alpha + data.left,\
	# 					i["y"] * data.alpha + data.up,\
	# 					font = "time 10 bold",\
	# 					text = str(i["physical_node_id"]),\
	# 					fill = data.color[occupied_robot_id - 1])

def drawLockNode(canvas, data):
	for i in data.lock_nodes:
		canvas.create_oval(i["x"] * data.alpha + data.left - data.node_r,\
						i["y"] * data.alpha + data.up - data.node_r,\
						i["x"] * data.alpha + data.left + data.node_r,\
						i["y"] * data.alpha + data.up + data.node_r,\
						fill = "pink", width = 0)

def drawLockText(canvas, data):
	for i in data.lock_nodes:
		canvas.create_text(i["x"] * data.alpha + data.left,\
						i["y"] * data.alpha + data.up,\
						font = "time 5 bold",\
						text = str(i["physical_node_id"]),\
						fill = "black")

def redrawAll(canvas, data):
	drawLinks(canvas, data)
	drawNodes(canvas, data)
	drawText(canvas, data)
	drawLockNode(canvas, data)
	drawLockText(canvas,data)


####################################
# use the run function as-is
####################################

def run(width=300, height=300):
	def redrawAllWrapper(canvas, data):
		canvas.delete(ALL)
		canvas.create_rectangle(0, 0, data.width, data.height,
								fill='white', width=0)
		redrawAll(canvas, data)
		canvas.update()	

	def mousePressedWrapper(event, canvas, data):
		mousePressed(event, data)
		redrawAllWrapper(canvas, data)

	def keyPressedWrapper(event, canvas, data):
		keyPressed(event, data)
		redrawAllWrapper(canvas, data)

	def timerFiredWrapper(canvas, data):
		timerFired(data)
		redrawAllWrapper(canvas, data)
		# pause, then call timerFired again
		canvas.after(data.timerDelay, timerFiredWrapper, canvas, data)
	# Set up data and call init
	class Struct(object): pass
	data = Struct()
	data.width = width
	data.height = height
	data.timerDelay = 100 # milliseconds
	init(data)
	# create the root and the canvas
	root = Tk()
	canvas = Canvas(root, width=data.width, height=data.height)
	canvas.pack()
	# set up events
	root.bind("<Button-1>", lambda event:
							mousePressedWrapper(event, canvas, data))
	root.bind("<Key>", lambda event:
							keyPressedWrapper(event, canvas, data))
	timerFiredWrapper(canvas, data)
	# and launch the app
	root.mainloop()  # blocks until window is closed
	print("bye!")

run(1000 , 800 )
