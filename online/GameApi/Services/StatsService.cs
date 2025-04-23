using StackExchange.Redis;
using System.Text.Json;

namespace GameApi.Services
{

    public class PlayerStats
    {
        public int GamesWon { get; set; }
        public int GamesPlayed { get; set; }
        public int CubesPushed { get; set; }
        public int MaxCubesPushedInOneGame { get; set; }

    }

    public class StatsService
    {
        private readonly IDatabase _db;
        private readonly AchievementService _achievementService;

        public StatsService(IConnectionMultiplexer redis, AchievementService achievementService)
        {
            _db = redis.GetDatabase();
            _achievementService = achievementService;
        }

        public async Task SetStatsAsync(string username, PlayerStats newStats)
        {
            var existingJson = await _db.HashGetAsync("stats", username);

            PlayerStats updatedStats;

            if (existingJson.HasValue)
            {
                // Log the existing JSON to inspect what is being retrieved
                Console.WriteLine($"Retrieved JSON: {existingJson}");

                try
                {
                    var existingStats = JsonSerializer.Deserialize<PlayerStats>(existingJson!) ?? new PlayerStats();

                    // Log individual fields to check values
                    Console.WriteLine($"Existing GamesWon: {existingStats.GamesWon}, New GamesWon: {newStats.GamesWon}");
                    Console.WriteLine($"Existing GamesPlayed: {existingStats.GamesPlayed}, New GamesPlayed: {newStats.GamesPlayed}");
                    Console.WriteLine($"Existing CubesPushed: {existingStats.CubesPushed}, New CubesPushed: {newStats.CubesPushed}");

                    updatedStats = new PlayerStats
                    {
                        // Make sure to add these as numbers, even if they're stored as strings in Redis
                        GamesWon = existingStats.GamesWon + newStats.GamesWon,
                        GamesPlayed = existingStats.GamesPlayed + newStats.GamesPlayed,
                        CubesPushed = existingStats.CubesPushed + newStats.CubesPushed,
                        MaxCubesPushedInOneGame = Math.Max(existingStats.MaxCubesPushedInOneGame, newStats.MaxCubesPushedInOneGame)
                    };
                }
                catch (JsonException e)
                {
                    Console.WriteLine($"Error deserializing JSON: {e.Message}");
                    updatedStats = newStats; // Fallback to newStats if deserialization fails
                }
            }
            else
            {
                updatedStats = newStats;
            }

            var updatedJson = JsonSerializer.Serialize(updatedStats);
            await _db.HashSetAsync("stats", username, updatedJson);

            await _achievementService.EvaluateAchievementsFromStatsAsync(username);
        }

        public async Task<PlayerStats?> GetStatsAsync(string username)
        {
            var json = await _db.HashGetAsync("stats", username);
            return json.HasValue ? JsonSerializer.Deserialize<PlayerStats>(json!) : null;
        }
    }
}
