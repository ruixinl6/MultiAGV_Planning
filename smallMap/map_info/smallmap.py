import json
import redis
import math
import csv
import copy


class PhysicalNode:
	def __init__(self, pid, x_value, y_value, type):
		self.physical_node_id = int(pid)
		self.x = float(x_value)
		self.y = float(y_value)
		self.phy_adjacent_nodes = []
		self.logical_nodes = []
		self.l_lock_nodes = []
		self.type = type
	def __repr__(self):
		return "node_id = " + str(self.physical_node_id)\
				+ " | x = " + str(self.x)\
				+ " | y = " + str(self.y)\
				+ " | neighbour = " + str(self.phy_adjacent_nodes)\
				+ " | logical_nodes = " + str(self.logical_nodes)\
				+ " | lock_nodes = " + str(self.l_lock_nodes)\
				+ " | type = " + str(self.type)


class PhysicalLink:
	def __init__(self, p_link_id, from_pid, to_pid):
		self.physical_link_id = int(p_link_id)
		self.from_physical_node = int(from_pid)
		self.to_physical_node = int(to_pid)
		self.l_lock_nodes = []
	def __repr__(self):
		return "link_id = " + str(self.physical_link_id)\
				+ " | from_phy_node = " + str(self.from_physical_node)\
				+ " | to_phy_node = " + str(self.to_physical_node)\
				+ " | lock_nodes = " + str(self.l_lock_nodes)

class LogicalNode:
	def __init__(self, lid, pid, ad):
		self.logical_node_id = int(lid)
		self.physical_node_id = int(pid)
		self.l_adjacent_nodes = ad
	def __repr__(self):
		return "logical_node_id = " + str(self.logical_node_id)\
				+ " | physical_node_id = " + str(self.physical_node_id)\
				+ " | l_adjacent_nodes = " + str(self.l_adjacent_nodes)

# class LogicalLink:
# 	def __init__(self, l_link_id, p_link_id, from_lid, to_lid, turn="0"):
# 		self.logical_link_id = l_link_id
# 		self.physical_link_id = p_link_id
# 		self.from_logical_node = from_lid
# 		self.to_logical_node = to_lid
# 		self.travel_time = 1
# 		self.turn = turn

csv_file = csv.reader(open('smallmap.csv', 'r'))

r = redis.Redis(host="localhost", port=6379, db=0)

raw_map = []
for i in csv_file:
	raw_map.append(i)


PhysicalNodeList = []
nodeIndex = 0
for i in range(len(raw_map)):
	for j in range(len(raw_map[i])):
		if (raw_map[i][j] == ''):
			continue
		num = raw_map[i][j]
		if (num == "0"):
			continue
		n = PhysicalNode(nodeIndex, i, j, num)
		PhysicalNodeList.append(n)
		nodeIndex += 1

for i in range(len(PhysicalNodeList)):
	currX = PhysicalNodeList[i].x
	currY = PhysicalNodeList[i].y
	if (PhysicalNodeList[i].type != "0"):
		for j in PhysicalNodeList:
			if (j.physical_node_id == PhysicalNodeList[i].physical_node_id):
				continue
			if (abs(j.x - currX) == 1 and abs(j.y - currY) == 0):
				PhysicalNodeList[i].phy_adjacent_nodes.append(j.physical_node_id)
			if (abs(j.x - currX) == 0 and abs(j.y - currY) == 1):
				PhysicalNodeList[i].phy_adjacent_nodes.append(j.physical_node_id)


for i in range(len(PhysicalNodeList)):
	PhysicalNodeList[i].logical_nodes.append(PhysicalNodeList[i].physical_node_id)

for i in range(len(PhysicalNodeList)):
	currX = PhysicalNodeList[i].x
	currY = PhysicalNodeList[i].y
	for j in PhysicalNodeList:
			if (abs(j.x - currX) <= 0 and abs(j.y - currY) <= 0):
				PhysicalNodeList[i].l_lock_nodes.append(j.physical_node_id)


PhysicalLinkList = []
linkIndex = 0
for i in PhysicalNodeList:
	for j in i.phy_adjacent_nodes:
		l = PhysicalLink(linkIndex, i.physical_node_id, j)
		PhysicalLinkList.append(l)
		linkIndex += 1

for i in PhysicalLinkList:
	from_node = i.from_physical_node
	to_node = i.to_physical_node
	temp_lock_nodes = []
	for j in PhysicalNodeList:
		if (j.physical_node_id == from_node or j.physical_node_id == to_node):
			for k in j.l_lock_nodes:
				if (k not in temp_lock_nodes):
					temp_lock_nodes.append(k)
	i.l_lock_nodes = temp_lock_nodes


LogicalNodeList = []
nodeIndex = 0
for i in PhysicalNodeList:
	l = LogicalNode(i.physical_node_id, i.physical_node_id, copy.deepcopy(i.phy_adjacent_nodes))
	LogicalNodeList.append(l)

for i in PhysicalNodeList:
	print i

for i in PhysicalLinkList:
	print i

for i in LogicalNodeList:
	print i

#r.flushdb()
for i in PhysicalNodeList:
	a = dict()
	a["physical_node_id"] = i.physical_node_id
	a["x"] = i.x
	a["y"] = i.y
	a["phy_adjacent_nodes"] = i.phy_adjacent_nodes
	a["logical_nodes"] = i.logical_nodes
	a["lock_nodes"] = i.l_lock_nodes
	a["type"] = i.type
	r.rpush("physical_node_list", json.dumps(a, ensure_ascii = False))

for i in PhysicalLinkList:
	a = dict()
	a["physical_link_id"] = i.physical_link_id
	a["from_physical_node"] = i.from_physical_node
	a["to_physical_node"] = i.to_physical_node
	a["lock_nodes"] = i.l_lock_nodes
	r.rpush("physical_link_list", json.dumps(a, ensure_ascii = False))

for i in LogicalNodeList:
	a = dict()
	a["physical_node_id"] = i.physical_node_id
	a["logical_node_id"] = i.logical_node_id
	a["l_adjacent_nodes"] = i.l_adjacent_nodes
	r.rpush("logical_node_list", json.dumps(a, ensure_ascii = False))



