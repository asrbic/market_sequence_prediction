package btf;
import java.util.Iterator;

public class HLOCV {
	
	public String symbol;
	public String date;
	public double high;
	public double low;
	public double open;
	public double close;
	public long volume;
	
	public HLOCV(String symbol, String date, double high, double low, double open, double close, long volume) {
		this.symbol = symbol;
		this.date = date;
		this.high = high;
		this.low = low;
		this.open = open;
		this.close = close;
		this.volume = volume;
	}
	
	public HLOCV(Iterator<String> recIter) {
		this.symbol = recIter.next();
		this.date = recIter.next();
		this.high = Double.parseDouble(recIter.next());
		this.low = Double.parseDouble(recIter.next());;
		this.open = Double.parseDouble(recIter.next());;
		this.close = Double.parseDouble(recIter.next());;
		this.volume = Long.parseLong(recIter.next());;
	}

	@Override
	public String toString() {
		return "HLOCV [symbol=" + symbol + ", date=" + date + ", high=" + high + ", low=" + low + ", open=" + open
				+ ", close=" + close + ", volume=" + volume + "]";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		long temp;
		temp = Double.doubleToLongBits(close);
		result = prime * result + (int) (temp ^ (temp >>> 32));
		result = prime * result + ((date == null) ? 0 : date.hashCode());
		temp = Double.doubleToLongBits(high);
		result = prime * result + (int) (temp ^ (temp >>> 32));
		temp = Double.doubleToLongBits(low);
		result = prime * result + (int) (temp ^ (temp >>> 32));
		temp = Double.doubleToLongBits(open);
		result = prime * result + (int) (temp ^ (temp >>> 32));
		result = prime * result + ((symbol == null) ? 0 : symbol.hashCode());
		result = prime * result + (int) (volume ^ (volume >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		HLOCV other = (HLOCV) obj;
		if (Double.doubleToLongBits(close) != Double.doubleToLongBits(other.close))
			return false;
		if (date == null) {
			if (other.date != null)
				return false;
		} else if (!date.equals(other.date))
			return false;
		if (Double.doubleToLongBits(high) != Double.doubleToLongBits(other.high))
			return false;
		if (Double.doubleToLongBits(low) != Double.doubleToLongBits(other.low))
			return false;
		if (Double.doubleToLongBits(open) != Double.doubleToLongBits(other.open))
			return false;
		if (symbol == null) {
			if (other.symbol != null)
				return false;
		} else if (!symbol.equals(other.symbol))
			return false;
		if (volume != other.volume)
			return false;
		return true;
	}
	
}
