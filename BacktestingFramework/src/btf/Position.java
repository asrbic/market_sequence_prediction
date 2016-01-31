package btf;

public class Position {
	public String symbol;
	public String date;
	public byte direction;
	public double price;
	public long quantity;
	
	public Position(Action action, long quantity)
	{
		symbol = action.symbol;
		date = action.date;
		direction = (byte)Math.round(action.direction);
		price = action.price;
		this.quantity = quantity; 
	}
}
