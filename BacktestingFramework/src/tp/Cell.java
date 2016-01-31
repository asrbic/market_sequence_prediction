package tp;

import static tp.TemporalPooler.TIME_STEPS;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Cell {
	
	public static final int MAX_NUM_SEGMENTS = 128;
	
	public int x, y, i;
	public byte[] state;
	public List<DendriteSegment> dseg;
	public List<SegmentUpdate> segUpdates;
	
	public Cell() {
		state = new byte[TIME_STEPS];
		dseg = new ArrayList<DendriteSegment>();
		segUpdates = new ArrayList<SegmentUpdate>();
	}
	
	DendriteSegment getActiveSegment(int timeIndex, byte state)
	{//ASSUMES activeSynapseCount WILL BE CALCULATED IN PHASE 2
		Iterator<DendriteSegment> it = dseg.iterator();
		DendriteSegment best = it.next();
		DendriteSegment tempSeg;
		while(it.hasNext())
		{
			tempSeg = it.next();
			if((tempSeg.state[timeIndex]&state) > 0)
			{
				if(best.sequenceSegment ^ tempSeg.sequenceSegment)	//10 OR 01
				{//if only one is a sequenceSegment AND if the sequenceSegment is tempSeg them make tempSeg the best
					if(tempSeg.sequenceSegment)
						best = tempSeg;
				}
				else							//11 OR 00
				{//if either both are or are not sequenceSegments then make best the one with the highest activeSynapseCount
					if(tempSeg.activeSynapseCount[timeIndex] > best.activeSynapseCount[timeIndex])
						best = tempSeg;
				}
			}
		}
		return best;
	}
	
	DendriteSegment getBestMatchingSegment(int timeIndex, int minThreshold)
	{
		DendriteSegment tempSeg;
		DendriteSegment best = null;
		int bestCount = minThreshold-1;
		Iterator<DendriteSegment> it = dseg.iterator();
		while(it.hasNext()) //segIndex=0, tempSeg=(tempCell.dSeg);segIndex < tempCell->numSegments;++segIndex, ++tempSeg)
		{
			tempSeg = it.next();
			if(tempSeg.activeCellSynapseCount[timeIndex] > bestCount)
			{
				best = tempSeg;
				bestCount = tempSeg.activeCellSynapseCount[timeIndex];
			}
		}

		return best;
	}
	
	void adaptSegments(boolean positiveReinforcement, long maxSegmentSize, double permInc, double permDec)
	{
		//printf("\t%i segment updates queued\n",tempCell->numSegmentUpdates);
		for(SegmentUpdate tempSegUpdate : segUpdates)
		{
			if(tempSegUpdate.dSeg == null)
			{//add new segment
				
				if(dseg.size() == MAX_NUM_SEGMENTS)
				{
					//printf("Cell (%2i,%2i,%i) is full.\n",tempCell->x,tempCell->y,tempCell->i);
					continue;
				}
				DendriteSegment newSeg = new DendriteSegment(tempSegUpdate.sequenceSegment);
				tempSegUpdate.dSeg = newSeg;
				dseg.add(newSeg);
			}
			else
			{//segment should have existing active synapses, reinforce them
				DendriteSegment tempSeg = tempSegUpdate.dSeg;
				Iterator<Synapse> synUpdateIter = tempSegUpdate.activeSynapses.iterator();
				Synapse synUpdate = synUpdateIter.next();
				for(Synapse tempSynapse : tempSeg.synapses)//synapseIndex=0;synapseIndex<tempSeg->numSynapses;++synapseIndex)
				{//iterate through list of all synapses in the segment
				 //update them based on whether or not they are on the list of synapses active in the segmentUpdate
				 //elements are added to the activeSynapses list linearly so we can linearly step through all synapses and check if we've reached the next synapse in activeSynapses yet
					if(tempSynapse == synUpdate) 
					{//also null != tempSynapse
						if(positiveReinforcement) {
							tempSynapse.perm+=permInc;
						}
						else {
							tempSynapse.perm-=permDec;
						}						
						synUpdate = synUpdateIter.next();
					}
					else
					{
						if(positiveReinforcement)
							tempSynapse.perm-=permDec;
					}				
				}
			}
			DendriteSegment tempSeg = tempSegUpdate.dSeg;
			if(!tempSegUpdate.synapsesToCreate.isEmpty() && tempSeg.synapses.size() < maxSegmentSize)
			{//add (newSynapseCount - numActiveSynapses) synapses to segment randomly chosen from the set of cells with learnState=1 at time timeIndex
				for(Synapse synapseToCreate : tempSegUpdate.synapsesToCreate)
				{
					tempSeg.synapses.add(synapseToCreate);
									
				}			
			}
		}
		
	}
}
