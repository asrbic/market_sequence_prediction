package tp;

public class Column {
	public Cell[] cells;
	
	
	Cell getBestMatchingCell(int timeIndex, int minThreshold)
	{	
		Cell bestCell = cells[0];
		int best=-1;
		int bestIndex=-1;
		int bestSegIndex;
		int tempIndex;
		DendriteSegment bestSeg;
		for(int i = 0; i < cells.length; ++i)
		{
			Cell tempCell = cells[i];
			bestSeg = tempCell.getBestMatchingSegment(timeIndex, minThreshold);
			
			
			if(bestSeg != null)
			{
				if(bestSeg.activeCellSynapseCount[timeIndex] > best)
				{
					best = bestSeg.activeCellSynapseCount[timeIndex];
					bestCell = tempCell;
				}
			}
			
		}
		if(best < 0)
		{//TODO optimise this nested loop out by calculating it in phase 2
			//printf("\tNo matching segment found\n");
			best = Integer.MAX_VALUE;//tempCell.dseg.size();//MAX_NUM_SEGMENTS+1;
			//printf("\tseg counts: ");
			for(Cell tempCell : cells) 
			//for(int i = 0; i < numCells; ++i)
			{
				int segSize = tempCell.dseg.size();
				if(segSize < best)
				{
					bestCell = tempCell;
					best = segSize;//c.cells[i].numSegments;
				}
				//printf(" %i",c->cells[i].numSegments);
			}
			//printf("\n\tcellIndex: %i numSegs: %i\n",bestIndex,best);
		}
		return bestCell;
	}
}
