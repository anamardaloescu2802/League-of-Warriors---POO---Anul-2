package entities;

import entities.characters.Character;
import java.util.ArrayList;

public class Account {
    private Information information;
    private ArrayList<Character> characters;
    private int gamesPlayed;

    public Account(ArrayList<Character> characters, int gamesNumber, Information information) {
        this.information = information;
        this.characters = characters;
        this.gamesPlayed=gamesNumber;
    }

    public ArrayList<Character> getCharacters() {
        return characters;
    }

    public Information getInformation() {
        return information;
    }

    public String getName() {
        return information.getName();
    }

    @Override
    public String toString() {
        return "Contul jucătorului:\n" + information.toString();
    }
}
