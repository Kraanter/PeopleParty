import type { MiniGamePayloadType } from "@/flatbuffers/mini-game-payload-type"

// NEW schema imports — adjust paths to your generated files
import { MarbleManiaHostPayload } from "@/flatbuffers/marble-mania-host-payload"
import { FBEntityType } from "@/flatbuffers/fbentity-type"
import { FBShape } from "@/flatbuffers/fbshape"
import { FBCircle } from "@/flatbuffers/fbcircle"
import { FBRect } from "@/flatbuffers/fbrect"
import { FBPoly } from "@/flatbuffers/fbpoly"
// end new schema imports

import type {
  MarbleManiaData,
  MarbleManiaEntity,
  MarbleManiaPlayerData,
  MarbleManiaResult,
  MarbleManiaResultPair,
  MMShape
} from "./MarbleManiaModels"

import { MarbleManiaResultPayload } from "@/flatbuffers/marble-mania-result-payload"
import { MarbleManiaPlayerPayload } from "@/flatbuffers/marble-mania-player-payload"

export function parseMarbleManiaHostPayload(data: MiniGamePayloadType): MarbleManiaData {
  const payload = data.gamestatepayload(new MarbleManiaHostPayload())

  const entities: MarbleManiaEntity[] = []
  for (let i = 0; i < payload.entitiesLength(); i++) {
    const e = payload.entities(i)!

    // type
    const typeEnum = e.type()
    const type: MarbleManiaEntity["type"] =
      typeEnum === FBEntityType.Marble ? "marble" : "obstacle"

    // center position
    const posObj = e.pos()!
    const pos = { x: posObj.x(), y: posObj.y() }

    // union shape
    let shape: MMShape | null = null
    switch (e.shapeType()) {
      case FBShape.FBCircle: {
        const c = e.shape(new FBCircle())!
        shape = { kind: "circle", radius: c.radius() }
        break
      }
      case FBShape.FBRect: {
        const r = e.shape(new FBRect())!
        shape = { kind: "rect", width: r.width(), height: r.height() }
        break
      }
      case FBShape.FBPoly: {
        const p = e.shape(new FBPoly())!
        const verts: { x: number; y: number }[] = []
        for (let vi = 0; vi < p.verticesLength(); vi++) {
          const v = p.vertices(vi)!
          verts.push({ x: v.x(), y: v.y() })
        }
        shape = { kind: "poly", vertices: verts }
        break
      }
      default:
        // unknown/none — skip
        continue
    }

    entities.push({
      id: e.id(),
      type,
      pos,
      rotation: e.rotation(),
      finished: e.finished(),
      player_name: e.playerName() ? decodeURI(e.playerName()!) : undefined,
      shape: shape!,
    })
  }

  const wm = payload.worldMin()!
  const wx = payload.worldMax()!

  return {
    entities,
    game_phase: payload.gamePhase(),
    placement_time_left: payload.placementTimeLeft(),
    finish_line_y: payload.finishLineY(),
    world_min: { x: wm.x(), y: wm.y() },
    world_max: { x: wx.x(), y: wx.y() }
  }
}

export function parseMarbleManiaPlayerPayload(data: MiniGamePayloadType): MarbleManiaPlayerData {
  const payload: MarbleManiaPlayerPayload = data.gamestatepayload(new MarbleManiaPlayerPayload())

  return {
    id: payload.id(),
    game_phase: payload.gamePhase(),
    placement_time_left: payload.placementTimeLeft(),
    y_pos: payload.yPos(),
    finish_line_y: payload.finishLineY()
  }
}

export function parseMarbleManiaResultPayload(data: MiniGamePayloadType): MarbleManiaResult {
  const payload: MarbleManiaResultPayload = data.gamestatepayload(new MarbleManiaResultPayload())

  const results: MarbleManiaResultPair[] = []
  for (let i = 0; i < payload.resultsLength(); i++) {
    results.push({
      name: decodeURI(payload.results(i)!.name()),
      placement: payload.results(i)!.placement(),
      time_to_finish: payload.results(i)!.timeToFinish(),
      has_finished: payload.results(i)!.hasFinished()
    })
  }

  results.sort((a, b) => a.placement - b.placement)
  return { results }
}
