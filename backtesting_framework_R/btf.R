#Code, Date, Open, High, Low, Close, Volume.
library(data.table)

#holdings <- main()
#print(holdings)

readCSVMarketData <- function(dir="/home/asrbic/views/market_sequence_prediction/test_data/data/") {
  allFileNames <- list.files(dir)
  colNames <- c("Code","Date","Open","High","Low","Close","Volume")
  all <- lapply(allFileNames, FUN = readSingleCSV, dir, colNames)
}

readSingleCSV <- function(fileName, dir, colNames) {
  singleCSV <- read.csv(paste0(dir, fileName), header=FALSE, col.names = colNames)
}

main <- function(params =list('OpenPositionSize' = 10000)) {
  set.seed(7)
  marketDataByInterval <- readCSVMarketData()
  openPositions <- data.frame("symbol"= character(), "price"= double(), "quantity" = integer(), "direction" = integer())
  cash <- 0
  for(interval in marketDataByInterval) {
    actions <- simulateInterval(interval)
    result <- takeActions(actions, interval, cash, openPositions, params)
    openPositions <- result$openPositions
    cash <- result$cash
  }
  #determine profit/loss given action dictated by indicators
  print(cash)
  openPositions
}

takeActions <- function(actions, interval, cash, openPositions, params) {
  actionsRequired <- data.frame(rbindlist(mapply(determineAction, actions, interval$Code, interval$High, interval$Low)))
  actionsRequired <- actionsRequired[!unlist(lapply(actionsRequired, is.null))]
  if(length(actionsRequired) > 0) {
    closeActionIndices <- actionsRequired[,c('symbol', 'direction')] %in% openPositions[,c('symbol', 'direction')]
    closeActions <- actionsRequired[closeActionIndices]
    createActions <- actionsRequired[!closeActionIndices]
    
    result <- closePositions(closeActions, openPositions, cash, params)
    cash <- result$cash
    openPositions <- result$openPositions
    result <- openNewPositions(createActions, openPositions, cash, params)
  }
  list("openPositions" = openPositions, "cash" = cash)
}

closePositions <- function(actions, openPositions, cash, params) {
  
  
  list("openPositions" = openPositions, "cash" = cash)
}

openNewPositions <- function(actions, openPositions, cash, params) {
  
  floor(params("OpenPositionSize"))
  
  list("openPositions" = openPositions, "cash" = cash)
}

determineAction <- function(action, symbol, high, low) {
  if(action < 0) { #SELL
    buildFrame(action, symbol, high)
  }
  else if(action > 0) { #BUY
    buildFrame(action, symbol, low)
  }
  else { #HOLD
    NULL
  }
}

buildFrame <- function(action, symbol, price) {
  data.frame("direction" = action, "symbol" = symbol, "price" = price)
}

simulateInterval <- function(interval) {
  #generate HTM features
  #this feature set should simulate one timeIndex in the TP
  #as if all the previous timeIndexes had taken place
  #might have to persist HTM state in another thread or something fancy
  
  #classify features
  #determine indicator (buy/sell/hold)
  
  #for now just randomly decide what to do
  rnd <- runif(nrow(interval), 0, 20)
  action <- lapply(rnd, randActionSelector)
  # -1 SELL | 0 HOLD | 1 BUY
}


randActionSelector <- function(num) {
  if(num < 1) {
    -1 #SELL
  }
  else if (num > 19) {
    1 #BUY
  }
  else {
    0 #HOLD
  }
}