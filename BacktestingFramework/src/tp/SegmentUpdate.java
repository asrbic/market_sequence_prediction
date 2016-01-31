package tp;

import java.util.ArrayList;
import java.util.List;

import static tp.TemporalPooler.ACTIVE_STATE;

public class SegmentUpdate {
	//public int segIndex;
	public boolean sequenceSegment;
	public List<Synapse> activeSynapses;
	public List<Synapse> synapsesToCreate;
	public DendriteSegment dSeg;
	//int activeSynapseIndices[MAX_SEGMENT_SIZE];
	//public synapse activeSynapses[MAX_SEGMENT_SIZE];
	public SegmentUpdate(DendriteSegment dSeg, boolean sequenceSegment, boolean newSynapses) {
		this.dSeg = dSeg;
		this.sequenceSegment = sequenceSegment;
		activeSynapses = new ArrayList<Synapse>();
		if(newSynapses) {
			synapsesToCreate = new ArrayList<Synapse>();
		}
	}
}
