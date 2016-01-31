package tp;

import static tp.TemporalPooler.TIME_STEPS;

import java.util.ArrayList;
import java.util.List;

public class DendriteSegment {
	byte state[];
	boolean sequenceSegment;
	int[] activeSynapseCount;
	int[] activeCellSynapseCount;
	List<Synapse> synapses;
	
	public DendriteSegment(boolean sequenceSegment) {
		state = new byte[TIME_STEPS];
		activeSynapseCount = new int[TIME_STEPS];
		activeCellSynapseCount = new int[TIME_STEPS];
		synapses = new ArrayList<Synapse>();
		this.sequenceSegment = sequenceSegment;
	}
	
	boolean segmentActive(int timeIndex, byte state)
	{//ASSUMES SEGMENT ACTIVE STATUS FLAGS WILL BE SET WHEN NECESSARY (PROBABLY PHASE 2)
		return (this.state[timeIndex]&state) > 0;
	}
	
}
