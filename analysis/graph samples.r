local({
  library(readr)
  library(ggplot2)
  library(Cairo)
  
  samples <- read_csv("volume s3 samples.txt", col_names=FALSE)
  samples.density <- density(samples$X1)
  
  samples.density.mode   <- samples.density$x[which.max(samples.density$y)]
  samples.density.median <- samples.density$x[min(which(cumsum(samples.density$y) > sum(samples.density$y) * 0.5))]
  
  df <- data.frame(value=samples.density$x, density=samples.density$y)
  
  p <<- ggplot(df, aes(x=value, y=density)) +
    scale_x_continuous(expand=c(0,0)) +
    scale_y_continuous(expand=c(0,0)) +
    geom_line() +
    geom_vline(xintercept=samples.density.mode,   color='red') +
    geom_vline(xintercept=samples.density.median, color='blue')
  
  dpi <- 72 * 1.5
  ggsave(plot=p, 'volume s3 samples.png', h=1080/dpi, w=1920/dpi, type="cairo-png", dpi=dpi)
  
  plot(p)
  print(paste("mode:", samples.density.mode))
  print(paste("median:", samples.density.median))
})