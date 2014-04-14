from subprocess import call
import threading

def runSim (W):
	N = 1
	while True:
		P = 0.0
		while P < 0.26:
			O="outputR-n"+str(N)+"-p"+str(P)+"-w"+str(W)
			call(["./waf", "--run", "scratch/mysecond --nCsma="+str(N)+" --dropP="+str(P)+" --winSize="+str(W)+" --o="+O])
			O="outputC-n"+str(N)+"-p"+str(P)+"-w"+str(W)
			call(["./waf", "--run", "scratch/mysecond --nCsma="+str(N)+" --dropP="+str(P)+" --winSize="+str(W)+" --JFRouter=false --o="+O])
			P += 0.05
					
		if N == 1:
			N = 2
		elif N == 2:
			N = 5
		elif N == 5:
			N = 10
		elif N == 10:
			N = 15
		elif N == 15:
			N = 20
		elif N == 20:
			N = 25
		else:
			break
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
