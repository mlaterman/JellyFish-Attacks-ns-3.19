from subprocess import call
from os import walk, remove
import threading

inDir = "tradPcap/"
outDir ="tCSV/"
step = 0.01
files = []
#DOES NOT WORK
def makeCSV (fList):
	for inFile in fList:
		outFile  = open(outDir+inFile[:-4]+"csv", "w+")
		tempFile = open(outDir+inFile[:-4]+"tmp", "w+")
		call(["tshark", "-o", "tcp.calculate_timestamps:true", "-T", "fields", "-e", "tcp.time_relative", "-e", "tcp.len", "-r", inDir+files[0]], stdout=tempFile)
		tempFile.close()
		tempFile = open(outDir+inFile[:-4]+"tmp", "r")
		ts = 0.0
		aBytes = 0
		oLine = ""
		for line in tempFile:
			stats = line.split()
			if len(stats) == 0:
				continue
			if float(stats[0]) < ts+step:
				aBytes += int(stats[1])
			else:
				oLine = str(ts)+","+str(aBytes)+"\n"
				outFile.write(oLine)
				aBytes = 0
				ts += step
		if oLine is not str(ts)+","+str(aBytes)+"\n":
			outFile.write(str(ts)+","+str(aBytes)+"\n")
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
