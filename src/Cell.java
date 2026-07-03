public class Cell {
    private int x, y;
    private CellEntityType type;
    private boolean visited;

    public Cell(int x, int y, CellEntityType type) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.visited = false;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public CellEntityType getType() {
        return type;
    }

    public void setType(CellEntityType type) {
        this.type = type;
    }

    public void visit() {
        this.visited = true;
    }

    @Override
    public String toString() {
        if (type == CellEntityType.PLAYER) {
            return "P";
        } else if (type == CellEntityType.VOID) {
            return "V";
        } else if (type == CellEntityType.ENEMY) {
            return "E";
        } else if (type == CellEntityType.SANCTUARY) {
            return "S";
        } else if (type == CellEntityType.PORTAL) {
            return "F";
        } else {
            return "N";
        }
    }


}
