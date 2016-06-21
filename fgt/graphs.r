report <- read.delim("C:/Users/Alexey/Projects/Colours visualization/fgt/report.tsv")
report$error <- abs(report$exact - report$approximate)

qplot(alpha, error, data=report, geom="jitter", color=I(alpha("black", 1/5)))

errors <- aggregate(error ~ alpha + n, data=report, FUN="mean")

qplot(n, error, data=errors, facets=.~alpha)