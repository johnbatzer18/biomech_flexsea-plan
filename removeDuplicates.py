import os

with open('plan-gui.pro', 'r') as f:
	content = f.readlines()

numLines = len(content)
print(numLines)

i = 0
while(i < len(content)-1):
	j = i + 1
	while(j < len(content)):
		line1 = content[i]
		line2 = content[j]
		if(line1.strip() == line2.strip() and line1.strip() != ""):
			content.pop(j)
		else:
			j = j + 1
	i = i + 1

fout = open('removedDuplicates.txt', 'w')
print("Writing out")
i = 0
while(i < len(content)):
	line = content[i]
	stripped = line.strip()
	if(stripped == ""):
		# line is a blank line
		fout.write("\n")
	elif(stripped != line):
		# line has some preceding / trailing white space
		fout.write("\t\t")
		fout.write(stripped)
	else:
		fout.write(line)
	i=i+1