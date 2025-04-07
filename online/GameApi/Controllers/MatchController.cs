using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [ApiController]
    [Route("matchmaking")]
    public class MatchController : ControllerBase
    {
        public static Dictionary<int, string> matchServers = new();
        private static int matchIdCounter = 1;

        [HttpPost("register")]
        public IActionResult RegisterServer([FromBody] MatchServer req)
        {
            matchServers[matchIdCounter++] = req.Ip;
            return Ok(new { Message = "Serveur enregistré", MatchId = matchIdCounter - 1 });
        }

        [Authorize]
        [HttpGet("find")]
        public IActionResult FindMatch()
        {
            var user = User.Identity.Name!;
            var playerStats = StatsController.stats.GetValueOrDefault(user, new PlayerStats());
            var level = playerStats.GamesWon; // match par parties gagnées

            // On prend le premier serveur dispo (simple pour commencer)
            if (matchServers.Count == 0)
                return NotFound("Aucun serveur dispo");

            var matchId = matchServers.Keys.First();
            return Ok(new { Ip = matchServers[matchId], MatchId = matchId });
        }
    }

    public class MatchServer
    {
        public string Ip { get; set; }
    }
}
