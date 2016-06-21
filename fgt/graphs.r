report <- read.delim("C:/Users/Alexey/Projects/Colours visualization/fgt/report.tsv")
report$error     <- abs(report$exact - report$approximate)
report$rel.error <- abs(report$exact - report$approximate) / report$exact

qplot(alpha, error, data=report, geom="jitter", color=I(alpha("black", 1/5)))

errors <- aggregate(error ~ alpha + n, data=report, FUN="mean")

qplot(n, error, data=errors, facets=.~alpha)

qplot(sigma, rel.error, data=report[report$alpha==5 & report$rel.error < 2,], color=I(alpha("black", 1/5)), facets=.~n)