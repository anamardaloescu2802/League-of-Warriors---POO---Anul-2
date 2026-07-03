import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import entities.Account;
import entities.Credentials;
import entities.Information;
import entities.characters.Character;
import entities.characters.CreateCharacters;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.SortedSet;
import java.util.TreeSet;

public class JsonInput {
    public static ArrayList<Account> deserializeAccounts() {
        String accountPath = "src/accounts.json"; // Asigurați-vă că calea este corectă
        try {
            // Citirea conținutului fișierului JSON
            String content = new String(Files.readAllBytes(Paths.get(accountPath)));
            JSONObject jsonObject = new JSONObject(content); // JSON-ul este un obiect care conține cheia "accounts"
            JSONArray accountsArray = jsonObject.getJSONArray("accounts"); // Accesăm lista de conturi

            ArrayList<Account> accounts = new ArrayList<>();
            for (int i = 0; i < accountsArray.length(); i++) {
                JSONObject accountJson = accountsArray.getJSONObject(i);

                // Citirea informațiilor din JSON
                String name = accountJson.getString("name");
                String country = accountJson.getString("country");
                int gamesNumber = accountJson.getInt("maps_completed");

                // Citirea credențialelor
                Credentials credentials = null;
                try {
                    JSONObject credentialsJson = accountJson.getJSONObject("credentials");
                    String email = credentialsJson.getString("email");
                    String password = credentialsJson.getString("password");
                    credentials = new Credentials(email, password);
                } catch (JSONException e) {
                    System.out.println("! Acest cont nu are toate credentialele !");
                }

                // Citirea jocurilor favorite
                SortedSet<String> favoriteGames = new TreeSet<>();
                try {
                    JSONArray games = accountJson.getJSONArray("favorite_games");
                    for (int j = 0; j < games.length(); j++) {
                        favoriteGames.add(games.getString(j));
                    }
                } catch (JSONException e) {
                    System.out.println("! Acest cont nu are jocuri favorite !");
                }

                // Construim obiectul Information folosind Builder-ul
                Information information = new Information.Builder()
                        .name(name)
                        .country(country)
                        .credentials(credentials)
                        .favoriteGames(favoriteGames)
                        .build();

                // Citirea și crearea personajelor
                ArrayList<Character> characters = new ArrayList<>();
                try {
                    JSONArray charactersListJson = accountJson.getJSONArray("characters");
                    for (int j = 0; j < charactersListJson.length(); j++) {
                        JSONObject charJson = charactersListJson.getJSONObject(j);
                        String cname = charJson.getString("name");
                        String profession = charJson.getString("profession");
                        int experience = charJson.getInt("experience");
                        int level = charJson.getInt("level");

                        // Folosim factory-ul pentru a crea personajul corespunzător
                        Character newCharacter = CreateCharacters.createCharacter(profession, cname, level, experience);

                        if (newCharacter != null) {
                            characters.add(newCharacter);
                        }
                    }
                } catch (JSONException e) {
                    System.out.println("! Acest cont nu are caractere !");
                }

                // Creăm obiectul Account și îl adăugăm în listă
                Account account = new Account(characters, gamesNumber, information);
                accounts.add(account);
            }
            return accounts;
        } catch (IOException | JSONException e) {
            e.printStackTrace();
        }
        return null;
    }
}
