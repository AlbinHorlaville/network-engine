using StackExchange.Redis;
using System.Text.Json;

namespace GameApi.Services
{
    public class AchievementService
    {
        private readonly IDatabase _db;

        public AchievementService(IConnectionMultiplexer redis)
        {
            _db = redis.GetDatabase();
        }

        public async Task AddTemplateAsync(string key, string description)
        {
            await _db.HashSetAsync("achievements", key, description);
        }

        public async Task<Dictionary<string, string>> GetTemplatesAsync()
        {
            var entries = await _db.HashGetAllAsync("achievements");
            return entries.ToDictionary(e => e.Name.ToString(), e => e.Value.ToString());
        }

        public async Task AddPlayerAchievementAsync(string username, string key, string description)
        {
            // Add the achievement to the player's achievements set
            await _db.SetAddAsync($"player:achievements:{username}", key);

            // Add the description to the achievements hash
            await _db.HashSetAsync("achievements", key, description);
        }

        public async Task EvaluateAchievementsFromStatsAsync(string username)
        {
            var json = await _db.HashGetAsync("stats", username);
            if (!json.HasValue) return;

            var stats = JsonSerializer.Deserialize<PlayerStats>(json!);
            if (stats == null) return;

            var alreadyUnlocked = await GetDetailedPlayerAchievementsAsync(username);

            // Example: define criteria here
            if (stats.GamesWon >= 1 && !alreadyUnlocked.ContainsKey("Winner"))
                await AddPlayerAchievementAsync(username, "Winner", "You won one game !");

            if (stats.CubesPushed >= 50 && !alreadyUnlocked.ContainsKey("Cube_Killer"))
                await AddPlayerAchievementAsync(username, "Cube_Killer", "Why would you do that ? They are so innocent.");

            if (stats.MaxCubesPushedInOneGame >= 90 && !alreadyUnlocked.ContainsKey("Smurf"))
                await AddPlayerAchievementAsync(username, "Smurf", "You owned them that game.");
        }

        public async Task<Dictionary<string, string>> GetDetailedPlayerAchievementsAsync(string username)
        {
            // Get the achievement keys for the player
            var keys = await _db.SetMembersAsync($"player:achievements:{username}");

            // If no achievements, return an empty dictionary
            if (keys.Length == 0)
                return new Dictionary<string, string>();

            // Retrieve descriptions for all the achievement keys in one batch
            var descriptions = await _db.HashGetAsync("achievements", keys.Select(k => (RedisValue)k).ToArray());

            // Create a dictionary to map keys to descriptions
            var result = new Dictionary<string, string>();
            for (int i = 0; i < keys.Length; i++)
            {
                var key = keys[i].ToString();
                var desc = descriptions[i].ToString();
                result[key] = desc;
            }
            return result;
        }
    }
}
