import entities.Credentials;
import java.util.Set;
import java.util.TreeSet;

public class Information {
    private Credentials credentials;
    private Set<String> favoriteGames;
    private String name;
    private String country;

    private Information(Builder builder) {
        this.credentials = builder.credentials;
        this.name = builder.name;
        this.country = builder.country;
        this.favoriteGames = builder.favoriteGames;
    }

    public Credentials getCredentials() {
        return credentials;
    }

    public String getName() {
        return name;
    }

    public String getCountry() {
        return country;
    }

    public Set<String> getFavoriteGames() {
        return favoriteGames;
    }

    public static class Builder {
        private Credentials credentials;
        private Set<String> favoriteGames = new TreeSet<>();
        private String name;
        private String country;

        public Builder setCredentials(Credentials credentials) {
            this.credentials = credentials;
            return this;
        }

        public Builder setName(String name) {
            this.name = name;
            return this;
        }

        public Builder setCountry(String country) {
            this.country = country;
            return this;
        }

        public Builder addFavoriteGame(String game) {
            this.favoriteGames.add(game);
            return this;
        }

        public Information build() {
            if (credentials == null || name == null || country == null) {
                throw new IllegalStateException("Credentials, name, and country are required!");
            }
            return new Information(this);
        }
    }
}
