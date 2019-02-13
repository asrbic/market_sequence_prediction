package crypto;

import java.util.List;

import org.canova.api.records.reader.SequenceRecordReader;
import org.deeplearning4j.datasets.canova.SequenceRecordReaderDataSetIterator;
import org.nd4j.linalg.dataset.DataSet;
import org.nd4j.linalg.dataset.api.DataSetPreProcessor;

public class PoloniexCandlestickDatasetIterator extends SequenceRecordReaderDataSetIterator {

	public PoloniexCandlestickDatasetIterator(SequenceRecordReader reader, int miniBatchSize, int numPossibleLabels,
			int labelIndex, boolean regression) {
		super(reader, miniBatchSize, numPossibleLabels, labelIndex, regression);
		// TODO Auto-generated constructor stub
	}
	
}
