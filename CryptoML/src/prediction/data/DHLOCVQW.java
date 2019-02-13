package prediction.data;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.TimeZone;
import java.util.spi.TimeZoneNameProvider;

import com.fasterxml.jackson.annotation.JsonProperty;

import sun.util.locale.provider.TimeZoneNameProviderImpl;

public class DHLOCVQW {
	private static String SEP = ",";
	
	public static final short LARGE_LOSS = 0;
	public static final short LOSS = 1;
	public static final short SMALL_LOSS = 2;
	public static final short NEUTRAL = 3;
	public static final short SMALL_GAIN = 4;
	public static final short GAIN = 5;
	public static final short LARGE_GAIN = 6;
	
	@JsonProperty
	long date;
	@JsonProperty
	double high;
	@JsonProperty
	double low;
	@JsonProperty
	double open;
	@JsonProperty
	double close;
	@JsonProperty
	double volume;
	@JsonProperty
	double quoteVolume;
	@JsonProperty
	double weightedAverage;
	
	public long getDate() {
		return date;
	}
	public void setDate(long date) {
		this.date = date;
	}
	public double getHigh() {
		return high;
	}
	public void setHigh(double high) {
		this.high = high;
	}
	public double getLow() {
		return low;
	}
	public void setLow(double low) {
		this.low = low;
	}
	public double getOpen() {
		return open;
	}
	public void setOpen(double open) {
		this.open = open;
	}
	public double getClose() {
		return close;
	}
	public void setClose(double close) {
		this.close = close;
	}
	public double getVolume() {
		return volume;
	}
	public void setVolume(double volume) {
		this.volume = volume;
	}
	public double getQuoteVolume() {
		return quoteVolume;
	}
	public void setQuoteVolume(double quoteVolume) {
		this.quoteVolume = quoteVolume;
	}

	public double getWeightedAverage() {
		return weightedAverage;
	}
	
	public void setWeightedAverage(double weightedAverage) {
		this.weightedAverage = weightedAverage;
	}
	
	public DHLOCVQW(long date, double high, double low, double open, double close, double volume, double quoteVolume, double weightedAverage) {
		this.date = date;
		this.high = high;
		this.low = low;
		this.open = open;
		this.close = close;
		this.volume = volume;
		this.quoteVolume = quoteVolume;
		this.weightedAverage = weightedAverage;
	}
	
	public DHLOCVQW() {
		this.date = 0;
		this.high = 0d;
		this.low = 0d;
		this.open = 0d;
		this.close = 0d;
		this.volume = 0d;
		this.quoteVolume = 0d;
		this.weightedAverage = 0d;
	}
	
	@Override
	public String toString() {
		return "DHLOCVQW [date=" + date + ", high=" + high + ", low=" + low + ", open=" + open + ", close=" + close
				+ ", volume=" + volume + ", quoteVolume=" + quoteVolume + ", weightedAverage=" + weightedAverage + "]";
	}
	
	public String getCSVLine() {
		return new StringBuilder()
				.append(high)
				.append(SEP)
				.append(low)
				.append(SEP)
				.append(open)
				.append(SEP)
				.append(close)
				.append(SEP)
				.append(volume)
				.append(SEP)
				.append(quoteVolume)
				.append(SEP)
				.append(weightedAverage)
				.toString();
	}
	
	public DHLOCVQW getDiff(DHLOCVQW other)
	{
		DHLOCVQW diff = new DHLOCVQW();
		diff.setDate(getDate() - other.getDate());
		diff.setHigh(percentDiffNum(getHigh(), other.getHigh()));
		diff.setLow(percentDiffNum(getLow(), other.getLow()));
		diff.setOpen(percentDiffNum(getOpen(), other.getOpen()));
		diff.setClose(percentDiffNum(getClose(), other.getClose()));
		diff.setVolume(percentDiffNum(getVolume(), other.getVolume())); //probs don't do this
		diff.setQuoteVolume(percentDiffNum(getQuoteVolume(), other.getQuoteVolume()));
		diff.setWeightedAverage(percentDiffNum(getWeightedAverage(), other.getWeightedAverage()));		
		
		return diff;
	}
	
	public double[] getHarmonicTime()
	{
		Calendar cal = new GregorianCalendar();
		cal.setTime(new Date(date*1000));
		int val = cal.get(Calendar.HOUR_OF_DAY) * 60 + cal.get(Calendar.MINUTE);
		int minutesInDay = 24*60;
		return new double[] {
				toHarmonicA(val, minutesInDay),
				toHarmonicB(val, minutesInDay)
		};
	}
	
	public short getWeightedAverageLabel() {
		if(weightedAverage < -0.05) {
			return LARGE_LOSS;
		}
		else if(weightedAverage < -0.02) {
			return LOSS;
		}
		else if(weightedAverage < -0.005) {
			return SMALL_LOSS;
		}
		else if(weightedAverage < 0.005) {
			return NEUTRAL;
		}
		else if(weightedAverage < 0.02) {
			return SMALL_GAIN;
		}
		else if(weightedAverage < 0.05) {
			return GAIN;
		}
		else {
			return LARGE_GAIN;
		}
	}
	
	
	public static double toHarmonicA(int val, int max)
	{
		return Math.sin(((double)val/(double)max) * Math.PI) - 1;
	}
	
	public static double toHarmonicB(int val, int max)
	{
		return Math.sin(2*((double)val/(double)max) * Math.PI) - 1;
	}
	
	public static double percentDiffNum(double a, double b)
	{
		return (a-b)/b;
	}
}
