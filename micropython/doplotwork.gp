set datafile sep ','
set yrange [0:3.5]
scy(idx,v)=v * ((idx==0) ? 0.75 : ((idx==1) ? 1.05 : ((idx==2) ? 1.3:2.5)))
set term qt 0
plot for [idx=0:3] 'results2.csv' index idx using ($6*3.3/1024):(scy(idx,($3)/4096.)):(column(-2)) lc variable
set term qt 1
plot for [idx=0:3] 'results2.csv' index idx using ($6*3.3/1024):(scy(idx,($4)/65536.)):(column(-2)) lc variable
set term qt 2
plot for [idx=0:3] 'results2.csv' index idx using ($6*3.3/1024):($5/1000000):(column(-2)) lc variable

