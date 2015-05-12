library("gplots")

args <- commandArgs(trailingOnly = TRUE)
data<-read.csv(paste(args[1], ".csv", sep=""))
pdf(paste(args[1], ".pdf", sep=""))
heatmap.2(as.matrix(data), dendrogram="none", Rowv=FALSE, Colv=FALSE, trace="none")
dev.off()
