package btf;

import static btf.BackTest.ACTIONABLE_LONG;
import static btf.BackTest.ACTIONABLE_SHORT;

public class Action {
	
	public String symbol;
	public String date;
	public double direction;
	public double price;
	
	public Action(String symbol, String date, double direction, double price) {
		this.symbol = symbol;
		this.date = date;
		this.direction = direction;
		this.price = price;
	}
	
	public Action(HLOCV entry, double direction) {
		this.symbol = entry.symbol;
		this.date = entry.date;
		this.direction = direction;
		if(direction >= ACTIONABLE_LONG) {
			this.price = entry.low;
		}
		else if(direction <= ACTIONABLE_SHORT) {
			this.price = entry.high;
		}
	}
	
}
