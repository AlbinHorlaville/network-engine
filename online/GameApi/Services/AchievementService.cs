using StackExchange.Redis;

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

        public async Task AddPlayerAchievementAsync(string username, string key)
        {
            await _db.SetAddAsync($"player:achievements:{username}", key);
        }

        public async Task<List<string>> GetPlayerAchievementsAsync(string username)
        {
            var values = await _db.SetMembersAsync($"player:achievements:{username}");
            return values.Select(v => v.ToString()).ToList();
        }
    }
}
