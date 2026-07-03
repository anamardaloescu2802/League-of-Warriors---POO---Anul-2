package entities.characters;

import entities.Entity;

public abstract class Character extends Entity {
    private String name;
    private int level;
    private int experience;
    private int health;
    private int mana;
    private int charisma;
    private int dexterity;
    private int strength;


    public Character(String name, int level, int experience, int mana, int health, int charisma, int dexterity, int strength) {
        this.name = name;
        this.experience = experience;
        this.level = level;
        this.health = health;
        this.mana = mana;
        this.charisma = charisma;
        this.dexterity = dexterity;
        this.strength = strength;
    }

    public String getType() {
        if (this.health == 120) {
            return "Mage";
        } else if (this.health == 60) {
            return "Warrior";
        } else if (this.health == 80) {
            return "Rogue";
        } else {
            return "Unknown";
        }
    }
    public void gainExperience(int n){
       this.experience += n;
    }
    public void levelUp(){
        this.level+=1;
    }
    public String getName() {
        return name;
    }

    public int getLevel() {
        return level;
    }

    public int getExperience() {
        return experience;
    }

    public void printDetails() {
        System.out.println("Abilitatile caracterului:\n" + "Carisma " + charisma + " | Dexteritate " + dexterity + " | Putere " + strength);
    }

    public int getHealth() {
        return health;
    }
    public void decreaseHealth(int amount) {
        this.health = Math.max(0, health - amount); // Asigură-te că sănătatea nu devine negativă
    }

    public int getMana() {
        return mana;
    }

    public int getCharisma() {
        return charisma;
    }

    public void setCharisma(int charisma) {
        this.charisma = charisma;
    }

    public int getDexterity() {
        return dexterity;
    }

    public void setDexterity(int dexterity) {
        this.dexterity = dexterity;
    }

    public int getStrength() {
        return strength;
    }

    public void setStrength(int strength) {
        this.strength = strength;
    }

    public abstract int calculateDamage();

    public void setMana(int min) {
        this.mana = min;
    }

    public void increaseHealth(int i) {
        this.health+=i;
    }


    public void increaseMana(int i) {
        this.mana += i;
    }
}
