from subprocess import call
import threading

def runSim (W):
	N = 1
	while N < 6:
		P = 0.0
		while P < 0.26:
			O="outputMob-n"+str(N)+"-p"+str(P)+"-w"+str(W)
			call(["./waf", "--run", "scratch/mobileNetwork --nJF="+str(N)+" --dropP="+str(P)+" --winSize="+str(W)+" --o="+O])
			P += 0.05
					
		N += 1
	print "Done Function"
			
t1 = threading.Thread(target=runSim, args=("0") )
t2 = threading.Thread(target=runSim, args=("3") )
t3 = threading.Thread(target=runSim, args=("5") )
t4 = threading.Thread(target=runSim, args=("10"))

t1.start()
t2.start()
t3.start()
t4.start()

t1.join()
t2.join()
t3.join()
t4.join()

print "Done All"
