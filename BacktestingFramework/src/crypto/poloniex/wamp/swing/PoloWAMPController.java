package crypto.poloniex.wamp.swing;

import java.awt.Button;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.swing.event.TableColumnModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableColumnModel;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableColumnModel;
import javax.swing.table.TableColumn;
import javax.swing.table.TableModel;

import com.fasterxml.jackson.databind.node.ArrayNode;

import crypto.poloniex.wamp.engine.PoloWAMPEngine;
import rx.functions.Action1;
import ws.wamp.jawampa.PubSubData;
import ws.wamp.jawampa.WampClient;

public class PoloWAMPController implements ActionListener, Action1 {
	public PoloWAMPGUI gui;
	public PoloWAMPEngine engine;
	public ExchangeTableModel model;
	public TableColumnModel colModel;
	public Map<String, Integer> makerCurrencies;
	public Map<String, Integer> currencies;
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
	
	public static final int MAKER_CURRENCY_SIDE_INDEX = 1;
	public static final int NORMAL_CURRENCY_SIDE_INDEX = 0;
	
	public static void main(String[] args) {
		PoloWAMPController controller = new PoloWAMPController();
		
	}
		
	public PoloWAMPController() {
		makerCurrencies = new HashMap<String, Integer>();
		currencies = new HashMap<String, Integer>();
		colModel = new DefaultTableColumnModel();
		
		model = new ExchangeTableModel();
		for(int i = 0; i < MAKER_CURRENCIES.length; ++i) {
			getTableIndexOfCurrency(MAKER_CURRENCIES[i]);
			getTableIndexOfMakerCurrency(MAKER_CURRENCIES[i]);
//			makerCurrencies.put(MAKER_CURRENCIES[i], i+1);
//			currencies.put(MAKER_CURRENCIES[i], i+1);
//			model.setRowName(i, MAKER_CURRENCIES[i]);
//			model.setColName(i, MAKER_CURRENCIES[i]);
		}
		
		
		gui = new PoloWAMPGUI(this, model, colModel);
		gui.setVisible(true);
		engine = new PoloWAMPEngine(this);
	}
	
	public void startPriceStream() {
		engine.doConnect();
	}
	
	public void stopPriceStream() {
		engine.close();
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		
		Object obj = e.getSource();
		if(obj instanceof Button) {
			switch (((Button) obj).getName()) {
			case "START":
				this.startPriceStream();
				break;
			case "STOP":
				this.stopPriceStream();
				break;
				
			}
		}
	}
	
	private void handleStateData(WampClient.State newState) {
		if (newState instanceof WampClient.ConnectedState) {
	          // Client got connected to the remote router
	          // and the session was established
			gui.connectionStatus.setText("Connected");
			engine.startTickerSub();
	        } else if (newState instanceof WampClient.DisconnectedState) {
	          // Client got disconnected from the remoute router
	          // or the last possible connect attempt failed
	        	gui.connectionStatus.setText("Disconnected");
	        } else if (newState instanceof WampClient.ConnectingState) {
	          // Client starts connecting to the remote router
	        	gui.connectionStatus.setText("Connecting...");
	        }
	}
	
	private void handlePubSubData(PubSubData data) {
		//System.out.println((data.details().toString()));
		ArrayNode args = data.arguments();
		String exchange = args.get(CURRENCY_PAIR_INDEX).asText();
		String[] sides = exchange.split("_");
		int currencyIndex = getTableIndexOfCurrency(sides[NORMAL_CURRENCY_SIDE_INDEX]);
		int makerCurrencyIndex = getTableIndexOfMakerCurrency(sides[MAKER_CURRENCY_SIDE_INDEX]);
		
		model.setValueAt(makerCurrencyIndex, currencyIndex, args.get(LAST_PRICE_INDEX).asDouble());
//		gui.tickerTable.repaint();
		//gui.comps.get(EXCHANGE_IDS[0]).setText(data.toString());
		//currencyPair, last, lowestAsk, highestBid, percentChange, baseVolume, quoteVolume, isFrozen, 24hrHigh, 24hrLow
	}

	private int getTableIndexOfCurrency(String currency) {
		if(!currencies.containsKey(currency)) {
			int insertionIndex = currencies.size() + 1;
			currencies.put(currency, insertionIndex);//i+1
			model.setRowName(insertionIndex, currency);
		}
		return currencies.get(currency);
	}
	
	private int getTableIndexOfMakerCurrency(String currency) {
		if(!makerCurrencies.containsKey(currency)) {
			int insertionIndex = makerCurrencies.size() + 1;
			makerCurrencies.put(currency, insertionIndex);//i+1
			model.setColName(insertionIndex, currency);
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
	
	public class ExchangeTableModel extends AbstractTableModel {
		/**
		 * 
		 */
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
	
	}
}
