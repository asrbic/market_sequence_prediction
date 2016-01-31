package tp;

public class Synapse {
	public float perm;
	public Cell connectedCell;
	
	public Synapse(Cell connectedCell, float perm) {
		this.connectedCell = connectedCell;
		this.perm = perm;
	}
}
