package crypto.poloniex.wamp.swing;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Observable;
import java.util.Observer;
import java.util.Set;


import com.fasterxml.jackson.databind.node.ArrayNode;

import crypto.poloniex.wamp.engine.PoloWAMPEngine;
import rx.functions.Action1;
import rx.schedulers.Schedulers;
import rx.subjects.BehaviorSubject;
import ws.wamp.jawampa.PubSubData;
import ws.wamp.jawampa.WampClient;

public class PoloWAMPController implements Action1 {
	public static final String[] MAKER_CURRENCIES = {"BTC", "XMR", "USDT"};
	//currencyPair, last, lowestAsk, highestBid, percentChange, baseVolume, quoteVolume, isFrozen, 24hrHigh, 24hrLow
	public static final int CURRENCY_PAIR_INDEX = 0;
	public static final int LAST_PRICE_INDEX = 1;
	public static final int LOWEST_ASK_INDEX = 2;
	public static final int HIGHEST_BID_INDEX = 3;
	public static final int PERCENT_CHANGE_INDEX = 4;
	public static final int BASE_VOLUME_INDEX = 5;
	public static final int QUOTE_VOLUME_INDEX = 6;
	public static final int IS_FROZEN_INDEX = 7;
	public static final int DAILY_HIGH_INDEX = 8;
	public static final int DAILY_LOW_INDEX = 9;
	
	public static final int MAKER_CURRENCY_SIDE_INDEX = 0;
	public static final int NORMAL_CURRENCY_SIDE_INDEX = 1;
	
	public PoloWAMPEngine engine;
	public Map<String, Integer> makerCurrencies;
	public Map<String, Integer> currencies;
	ConnectionStateObservable connectionStateObservable;
	TableCellObservable tableCellObservable;
	Map<String, TableRow> tableRows;
	List<String> colNames;
	
//	public static void main(String[] args) {
//		PoloWAMPController controller = new PoloWAMPController();
//		
//	}
		
	public PoloWAMPController() {
		makerCurrencies = new HashMap<String, Integer>();
		currencies = new HashMap<String, Integer>();
		colNames = new ArrayList<String>();
		for(int i = 0; i < MAKER_CURRENCIES.length; ++i) {
			getTableIndexOfCurrency(MAKER_CURRENCIES[i]);
			getTableIndexOfMakerCurrency(MAKER_CURRENCIES[i]);
			
			//TODO add generated cols to following
			colNames.add(MAKER_CURRENCIES[i]);
		}
		for(int i = 0; i < MAKER_CURRENCIES.length; ++i) {
			for(int j = i+1; j < MAKER_CURRENCIES.length; ++j) {
				colNames.add(MAKER_CURRENCIES[i] + "*" + MAKER_CURRENCIES[j]);
			}
		}
		connectionStateObservable = new ConnectionStateObservable();
		tableCellObservable = new TableCellObservable();
		tableRows = new HashMap<String, TableRow>();
		engine = new PoloWAMPEngine(this);
		
	}
	
	public void addConnectionStateobserver(Observer observer) {
		connectionStateObservable.addObserver(observer);
	}
	
	public void addTableCellObserver(Observer observer) {
		tableCellObservable.addObserver(observer);
	}
	
	public void startPriceStream() {
		engine.doConnect();
	}
	
	public void stopPriceStream() {
		engine.close();
	}
	
	public List<String> getColNames() {
		return colNames;
	}
	
	private void handleStateData(WampClient.State newState) {
		if (newState instanceof WampClient.ConnectedState) {
	          // Client got connected to the remote router
	          // and the session was established
//			gui.connectionStatus.setText("Connected");
			engine.startTickerSub();
	        } else if (newState instanceof WampClient.DisconnectedState) {
	          // Client got disconnected from the remoute router
	          // or the last possible connect attempt failed
//	        	gui.connectionStatus.setText("Disconnected");
	        } else if (newState instanceof WampClient.ConnectingState) {
	          // Client starts connecting to the remote router
//	        	gui.connectionStatus.setText("Connecting...");
	        }
		connectionStateObservable.setState(newState.toString());
	}
	
	private void handlePubSubData(PubSubData data) {
		//System.out.println((data.details().toString()));
		ArrayNode args = data.arguments();
		String exchange = args.get(CURRENCY_PAIR_INDEX).asText();
		String[] sides = exchange.split("_");
		String currencyCode = sides[NORMAL_CURRENCY_SIDE_INDEX];
		int currencyIndex = getTableIndexOfCurrency(currencyCode);
		int makerCurrencyIndex = getTableIndexOfMakerCurrency(sides[MAKER_CURRENCY_SIDE_INDEX]);
		
		boolean isNew = false;
		TableRow row = tableRows.get(currencyCode);
		if(row == null) {
			isNew = true;
			row = new TableRow(currencyCode, MAKER_CURRENCIES.length, MAKER_CURRENCIES.length *(MAKER_CURRENCIES.length - 1));
			tableRows.put(currencyCode, row);
		}
		double rate = args.get(LAST_PRICE_INDEX).asDouble();
		row.setRate(makerCurrencyIndex, rate);
		tableCellObservable.setTableCell(new TableCell(currencyCode, colNames.get(makerCurrencyIndex), rate), isNew);
		for(int i = 0; i < MAKER_CURRENCIES.length; ++i) {
			for(int j = i+1; j < MAKER_CURRENCIES.length; ++j) {
				String colName = MAKER_CURRENCIES[i] + "*" + MAKER_CURRENCIES[j];
				double compositeRate = getRateOfMakerCurrencies(i, j);
				tableCellObservable.setTableCell(new TableCell(currencyCode, colName, compositeRate), false);		
			}
		}
		//currencyPair, last, lowestAsk, highestBid, percentChange, baseVolume, quoteVolume, isFrozen, 24hrHigh, 24hrLow
	}
	
	private double getRateOfMakerCurrencies(int i, int j) {
		TableRow itr = tableRows.get(colNames.get(i));
		double rate = 0;
		if(itr != null) {
			rate = itr.getRate(j);  
		}
		if(rate == 0) {
			TableRow jtr = tableRows.get(colNames.get(j));
			if(jtr != null) {
				if(jtr.getRate(j) != 0) {
					rate = 1/jtr.getRate(j);
				}
			}
		}
		return rate;
	}

	private int getTableIndexOfCurrency(String currency) {
		if(!currencies.containsKey(currency)) {
			int insertionIndex = currencies.size();
			currencies.put(currency, insertionIndex);//i+1
//			model.setRowName(insertionIndex, currency);
		}
		return currencies.get(currency);
	}
	
	private int getTableIndexOfMakerCurrency(String currency) {
		if(!makerCurrencies.containsKey(currency)) {
			int insertionIndex = makerCurrencies.size();
			makerCurrencies.put(currency, insertionIndex);//i+1
//			model.setColName(insertionIndex, currency);
		}
		return makerCurrencies.get(currency);
	}

	@Override
	public void call(Object data) {
		// TODO Auto-generated method stub
		if(data instanceof WampClient.State) {
			handleStateData((WampClient.State)data);
		}
		else if(data instanceof PubSubData) {
			handlePubSubData((PubSubData)data);
		}
	}
	
	public class ConnectionStateObservable extends Observable {
		private String state;
		
		public ConnectionStateObservable() {state = null;}
		
		public void setState(String state) {
			this.state = state;
			setChanged();
			notifyObservers();
		}
		
		public String getState() {
			return state;
		}
	}
	
	public class TableCell {
		public String rowId;
		public String colId;
		public double value;
		public TableCell(String rowId, String colId, double value) {
			this.rowId = rowId;
			this.colId = colId;
			this.value = value;
		}
		
		
	}
	
	public class TableRow implements Serializable {
		
		private static final long serialVersionUID = 1L;
		String code;
		double[] rates;
		double[] calcs;
		
		public TableRow(String code, int rateSize, int calcSize) {
			this.code = code;
			rates = new double[rateSize];
			calcs = new double[calcSize];
		}
		
		public String getCode() {
			return code;
		}
		
		public void setCode(String code) {
			this.code = code;
		}
		
		public void setRate(int index, double value) {
			rates[index] = value;
		}
		
		public void setCalc(int index, double value) {
			calcs[index] = value;
		}
		
		public double getRate(int index) {
			return rates[index];
		}
		
		public double getCalc(int index) {
			return calcs[index];
		}
		
		public void setRates(double[] value) {
			rates = value;
		}
		
		public void setCalcs(double[] value) {
			calcs = value;
		}
		
		public double[] getCalcs() {
			return calcs;
		}
		
		public double[] getRates() {
			return rates;
		}
	}
	
	public class TableCellObservable extends Observable {
		private TableCell tc = null;
		private boolean isNew;
		
		public void setTableCell(TableCell tc, boolean isNew) {
			this.tc = tc;
			setChanged();
			notifyObservers();
		}
		
		public TableCell getTableCell() {
			return tc;
		}
		
		public boolean isNew() {
			return isNew;
		}
	}
	
	public class TableRowObservable extends Observable {
		private TableRow tr = null;
		private boolean isNew;
		
		public void setTableRow(TableRow tr, boolean isNew) {
			this.tr = tr;
			this.isNew = isNew;
			setChanged();
			notifyObservers();
		}
		
		public TableRow getTableRow() {
			return tr;
		}
		
		public boolean isNew() {
			return isNew;
		}
	}
	
	/*public class ExchangeTableModel extends AbstractTableModel {
		private static final long serialVersionUID = 1L;
		//col,row
		public List<List<Object>> tableData;
		
		
		public ExchangeTableModel() {
			super();
			tableData = new ArrayList<List<Object>>();
		}
		
		public void setColName(int index, String name) {
			setValueAt(index, 0, name);
		}
		
		public void setRowName(int index, String name) {
			setValueAt(0, index, name);
		}
		
		public synchronized void setValueAt(int row, int col, Object value) {
			
			List<Object> colList = null;
			if(col < tableData.size()) {
				colList = tableData.get(col);
			}
			if(colList == null) {
				colList = new ArrayList<Object>();
				boolean sizeChanged = insertNulls(tableData, col);
				tableData.set(col, colList);
				if(sizeChanged) {
					fireTableStructureChanged();
					fireTableDataChanged();
				}
			}
			boolean sizeChanged = insertNulls(colList, row);
			colList.set(row, value);
			if(sizeChanged) {
				fireTableStructureChanged();
				fireTableDataChanged();
			}
			fireTableCellUpdated(row, col);
		}
		
		private boolean insertNulls(List list, int toIndex) {
			if(toIndex < list.size()) {
				return false;
			}
			for(int i = list.size(); i <= toIndex; ++i) {
				list.add(i, null);
			}
			return true;
		}
		
		@Override
		public int getRowCount() {
			return tableData.get(0).size();
		}

		@Override
		public int getColumnCount() {
			return tableData.size();
		}

		@Override
		public synchronized Object getValueAt(int rowIndex, int columnIndex) {
			List<Object> colList = null;
			if(columnIndex < tableData.size()) {
				colList = tableData.get(columnIndex);
			}
			if(colList == null) {
				return null;
			}
			if(rowIndex >= colList.size()) {
				return null;
			}
			return colList.get(rowIndex);
		}
	
	}*/
	

//	public class TableDataUpdate {
//
//
//		private final int row;
//		private final int col;
//		private final double value;
//		
//		public TableDataUpdate(int row, int col, double value) {
//			this.row = row;
//			this.col = col;
//			this.value = value;
//		}
//		
//		public int getRow() {
//			return row;
//		}
//		
//		public int getCol() {
//			return col;
//		}
//		
//		public double getValue() {
//			return value;
//		}
//		
//		@Override
//		public String toString() {
//			return "TableDataUpdate [row=" + row + ", col=" + col + ", value=" + value + "]";
//		}
//	}

}
