package entities.characters;

public class CreateCharacters {
    public static Character createCharacter(String profession, String name, int level, int experience) {
        switch (profession) {
            case "Warrior":
                return new Warrior(name, level, experience);
            case "Mage":
                return new Mage(name, level, experience);
            case "Rogue":
                return new Rogue(name, level, experience);
            default:
                throw new IllegalArgumentException("Unknown profession: " + profession);
        }
    }
}
