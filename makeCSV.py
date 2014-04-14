from os import walk, system
import threading

def makeCSV (fList):
	for inFile in fList:
		oName = inFile[:-4]+"csv"
		system("tshark -o tcp.calculate_timestamps:true -T fields -e tcp.time_relative -e tcp.len -E separator=, -r "+inDir+files[0]+" > "+outDir+oName)
	print "Done Function"

inDir = "tradPcap/"
outDir ="tradCSV/" 
files = []

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
