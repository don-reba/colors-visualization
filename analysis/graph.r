library(readr)
library(ggplot2)
library(Cairo)

GraphPerf <- function(path) {
  data <- read_table2(path, col_types = cols(threads = col_integer(), 
                                             time    = col_integer()))
  data$perf <- 1000 / data$time
  
  dpi <- 72 * 1.5
  
  plot <- ggplot(data, aes(threads, perf)) + stat_summary(aes(y=perf), fun.y=mean, colour="gray", geom="line") + geom_point()
  
  plot_path <- paste(tools::file_path_sans_ext(path), ".png", sep="")
  
  ggsave(plot=plot, plot_path, h=1080/dpi, w=1920/dpi, type="cairo-png", dpi=dpi)
}

GraphPerf("render perf (tr1920x).txt")
GraphPerf("voxel perf (tr1920x).txt")