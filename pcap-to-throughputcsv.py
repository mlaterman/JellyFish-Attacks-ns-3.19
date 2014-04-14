from subprocess import call
from os import walk, remove
import threading

inDir = "tradPcap/"
outDir ="tCSV/"
files = []
oList = []
step = 0.01

def makeCSV (fList):
	for inFile in fList:
		outFile = open(outDir+inFile[:-4]+"csv", "w+")
		tempFile = open(outDir+inFile[:-4]+"tmp", "w+")
		call(["captcp", "throughput", "-s", str(step), "--stdio", inDir+inFile], stdout=tempFile)
		tempFile.close()
		tempFile = open(outDir+inFile[:-4]+"tmp", "r")
		ts = 0;
		for line in tempFile:
			stats = line.split() #Properly set the TS of each line
			line = str(ts+step)+","+stats[1]+"\n"
			ts+=step
			outFile.write(line)
		tempFile.close()
		outFile.close()
		remove(outDir+inFile[:-4]+"tmp")


for (dirpath, dirnames, filenames) in walk(inDir):
    files.extend(filenames)
    break
fLen = len(files)
t1 = threading.Thread(target=makeCSV, args =([files[0:fLen/4]]) )
t2 = threading.Thread(target=makeCSV, args =([files[fLen/4:fLen/2]]) )
t3 = threading.Thread(target=makeCSV, args =([files[fLen/2:fLen/2+fLen/4]]) )
t4 = threading.Thread(target=makeCSV, args =([files[fLen/2+fLen/4:]]) )

t1.start()
t2.start()
t3.start()
t4.start()

t1.join()
t2.join()
t3.join()
t4.join()

print "Done All"
