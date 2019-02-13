package prediction.util;

import java.io.File;
import java.io.PrintWriter;
import java.util.Date;
import java.util.List;
import java.util.Random;

import org.apache.commons.math3.linear.RectangularCholeskyDecomposition;

import com.fasterxml.jackson.core.JsonFactory;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.databind.MappingIterator;
import com.fasterxml.jackson.databind.ObjectMapper;

import prediction.data.DHLOCVQW;

public class StaticDataPrep {
	
	public static void main(String[] args) throws Exception
	{
		
		String baseDir = "C:\\\\Users/asrbic/repos/market_sequence_prediction/polodata/";
		//read json, write csv for each sequence ideally
		String dataPath = baseDir + "BTC_ETH_300";
		
	    final ObjectMapper mapper = new ObjectMapper();
	    JsonParser jp = new JsonFactory().createParser(new File(dataPath));
	    DHLOCVQW[] candles = mapper.readValue(jp, DHLOCVQW[].class);
	    
	    

	    Random rand = new Random(7);
	    String featureDir = baseDir + "sequences/features/";
	    String labelDir = baseDir + "sequences/labels/";
	    String rawDir = baseDir + "sequences/raw/";
	    // super sample
	    int seqLength = 20;
	    int offset = 10;
	    int dataExampleId = 0;
	    for(int i = 0; i < candles.length - seqLength; i+=offset)
	    {
	    	File rawFile = new File(rawDir + dataExampleId +  ".csv");
	    	PrintWriter rawWriter = new PrintWriter(rawFile);
	    	
		    File featureFile = new File(featureDir + dataExampleId +  ".csv");
		    PrintWriter featureWriter = new PrintWriter(featureFile);
		    
	    	File labelFile = new File(labelDir + dataExampleId++ +  ".csv");
		    PrintWriter labelWriter = new PrintWriter(labelFile);
		    
		    DHLOCVQW diff;
		    if(i == 0)
		    {
		    	diff = new DHLOCVQW();	
		    }
		    else
		    {
		    	diff = candles[i].getDiff(candles[i-1]);
		    }
		    //init empty obj for for iter - there is no previous to diff to
	    	for(int j = 0;j < seqLength;++j)
	    	{
	    		featureWriter.println(diff.getCSVLine());
	    		labelWriter.println(diff.getWeightedAverageLabel());
	    		rawWriter.println(candles[i+j].getCSVLine());
	    		diff = candles[i+j+1].getDiff(candles[i+j]);
	    	}
	    	
	    	//verification
//	    	double[] harmonicTimes = candles[i].getHarmonicTime();
//	    	System.out.println(new Date(candles[i].getDate()*1000)*/i +"," + harmonicTimes[0] + "," + harmonicTimes[1]);
	    	
	    	featureWriter.close();
	    	labelWriter.close();
	    	rawWriter.close();

    	}
	    
	    
	    
	}
}
