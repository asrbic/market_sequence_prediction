package btf;
import java.io.File;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.apache.commons.csv.CSVFormat;
import org.apache.commons.csv.CSVParser;
import org.apache.commons.csv.CSVRecord;

public class BackTest {
	
	public static final byte SHORT = -1;
	public static final byte HOLD = 0;
	public static final byte LONG = 1;
	public static final long DEFAULT_BUY_SIZE = 5000;
	public static final double ACTIONABLE_LONG = 0.8d;
	public static final double ACTIONABLE_SHORT = -0.8d;
	
	List<List<HLOCV>> entries;
	Map<String, List<Position>> positions;
	String dataPath;
	double cash;
	
	public BackTest(String dataPath) {
		entries = new ArrayList<List<HLOCV>>();
		positions = new HashMap<String, List<Position>>();
		this.dataPath = dataPath;
	}
	
	public void run() {
		readCSVs();
		for(List<HLOCV> intervalEntries : entries) {
			List<Action> actions = getActions(intervalEntries);
			applyAction(actions);
		}
		System.out.println("CASH: $" + cash);
	}
	
	private List<Action> getActions(List<HLOCV> intervalEntries) {
		List<Action> actions = new ArrayList<Action>();
		for(HLOCV entry : intervalEntries) {
			double direction = getDirection(entry);
			if(direction > ACTIONABLE_LONG || direction == ACTIONABLE_SHORT) {
				actions.add(new Action(entry, direction));
			}
		}
		applyAction(actions);
		return actions;	
	}
	
	
	private double getDirection(HLOCV entry) {
		return getRandomDirection();
	}
	
	private double getRandomDirection() {
		return Math.random()*2-1;
	}
	
	private void applyAction(List<Action> actions) {
		for(Action action : actions) {
			String symbol = action.symbol;
			if(positions.containsKey(symbol)) {
				//close/update existing positions
				List<Position> positionsForSymbol = positions.get(symbol);
				for(Position pos : positionsForSymbol) {
					if(pos.direction != action.direction) {
						finalisePosition(pos, positionsForSymbol, action);
					}
				}
				// Need some metric to determine whether this should be done
				positionsForSymbol.add(openNewPosition(action));
			}
			else {
				//create new position list & position
				List<Position> positionsForSymbol = new LinkedList<Position>();
				positionsForSymbol.add(openNewPosition(action));
				positions.put(symbol, positionsForSymbol);				
			}
		}
	}
	
	private void finalisePosition(Position position, List<Position> positionsForSymbol, Action action) {
		double openPrice = position.price;
		double closePrice = action.price;
		if(position.direction > 0) {
			//selling out a long position
			cash += closePrice - openPrice;
		}
		else {
			//buying out a short position
			cash += openPrice - closePrice;
		}
		positionsForSymbol.remove(position);
	}
	
	private Position openNewPosition(Action action) {
		long quantity = getQuantityToTrade(action);
		double positionCost = quantity * action.price;
		cash -= positionCost;
		return new Position(action, quantity);
	}
	
	private long getQuantityToTrade(Action action) {
		return Math.round(5000/action.price);
	}
	
	private void readCSVs() {
		try {
			File dataDir = new File(dataPath);
			File[] csvs = dataDir.listFiles();
			Arrays.sort(csvs);
			for(File csv : csvs) {
				List<HLOCV> fileEntries = new ArrayList<HLOCV>();
			    CSVParser parser = CSVParser.parse(csv, Charset.defaultCharset(), CSVFormat.DEFAULT);
			    for(CSVRecord rec : parser) {
			    	HLOCV entry = new HLOCV(rec.iterator());
			    	fileEntries.add(entry);
			    }
			    entries.add(fileEntries);
			}
		} catch(Exception e) {e.printStackTrace();}
		System.out.println("DONE CSV READ");
	}
}
