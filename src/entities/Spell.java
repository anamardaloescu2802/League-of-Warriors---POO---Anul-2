package entities;

public abstract class Spell {
    private String type;
    private int damage;
    private int manaCost;

    public Spell(String type, int damage, int manaCost) {
        this.type = type;
        this.damage = damage;
        this.manaCost = manaCost;
    }

    public String getType() {
        return type;
    }

    public int getDamage() {
        return damage;
    }

    public int getManaCost() {
        return manaCost;
    }

    @Override
    public String toString() {
        return type + " (Damage: " + damage + ", Mana Cost: " + manaCost + ")";
    }
}
