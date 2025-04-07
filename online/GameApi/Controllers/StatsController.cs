using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [Authorize]
    [ApiController]
    [Route("stats")]
    public class StatsController : ControllerBase
    {
        public static Dictionary<string, PlayerStats> stats = new();

        [HttpGet]
        public IActionResult GetStats()
        {
            var user = User.Identity.Name!;
            if (!stats.ContainsKey(user))
                stats[user] = new PlayerStats();

            return Ok(stats[user]);
        }

        [HttpPost]
        public IActionResult UpdateStats([FromBody] PlayerStats newStats)
        {
            var user = User.Identity.Name!;
            stats[user] = newStats;
            return Ok(stats[user]);
        }
    }

    public class PlayerStats
    {
        public int GamesWon { get; set; }
        public int CubesCleared { get; set; }
    }
}