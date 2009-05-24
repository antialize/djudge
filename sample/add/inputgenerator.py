from random import randint,seed
seed(42)
for i in range(20):
    open("inputs/gen_%d"%i,"w").write("%d %d\n"%(randint(0,10000),randint(0,10000)))
