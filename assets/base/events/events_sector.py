from engine import create_logger, event_bus, EventPlayer  # noqa

logger = create_logger(__name__)


@event_bus.listen('sector_player_added')
def event_sector_player_added(event: EventPlayer):
    logger.info(f'event_sector_player_added() called with player: {event.player_id}')
