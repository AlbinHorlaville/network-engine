using GameApi.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [ApiController]
    [Authorize]
    [Route("[controller]")]
    public class MatchmakingController : ControllerBase
    {
        private readonly MatchmakingService _matchService;
        private readonly StatsService _statsService;

        public MatchmakingController(MatchmakingService matchService, StatsService statsService)
        {
            _matchService = matchService;
            _statsService = statsService;
        }

        [HttpPost("queue")]
        public async Task<IActionResult> QueuePlayer()
        {
            var username = User.Identity?.Name!;
            var stats = await _statsService.GetStatsAsync(username);
            var wins = stats?.GamesWon ?? 0;
            await _matchService.QueuePlayerAsync(username, wins);
            return Ok("Player queued");
        }

        [HttpGet("match")]
        public async Task<IActionResult> GetMatch([FromQuery] int tier)
        {
            var players = await _matchService.GetNextMatchAsync(tier);
            if (players.Count == 0) return NotFound("Not enough players");

            var server = await _matchService.GetAvailableServerAsync();
            if (server == null) return StatusCode(503, "No available server");

            return Ok(new { Server = server, Players = players });
        }

        [HttpPost("register")]
        public async Task<IActionResult> RegisterServer([FromQuery] string ip)
        {
            await _matchService.RegisterServerAsync(ip);
            return Ok("Server registered");
        }
    }
}
